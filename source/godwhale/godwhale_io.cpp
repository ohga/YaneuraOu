
#if defined GODWHALE_CLUSTER_SLAVE
#include <iostream>
#include <thread>
#include "godwhale_io.hpp"

#include <boost/asio/ip/tcp.hpp>

// boost1.66からインターフェースが新しくなり、バグも修正された。
#if BOOST_VERSION >= 106600
#include <boost/asio/basic_socket_streambuf.hpp>
#else
#include "asio_socket_streambuf.hpp"
#endif

namespace boost
{
    void throw_exception(std::exception const &e) { assert(false); }
}

// logging用のhack。streambufをこれでhookしてしまえば追加コードなしで普通に
// streambufの入出力を他のstreambufにリダイレクトできる。
// cf. http://groups.google.com/group/comp.lang.c++/msg/1d941c0f26ea0d81
struct Tee : public std::streambuf
{
    Tee(std::streambuf* buf_, std::streambuf* log_, bool log_is_out_)
        : buf(buf_), log(log_), prefix(log_is_out_ ? "<< " : ">> ") {}

    int sync() { return buf->pubsync(); }
    int_type uflow() { return buf->sbumpc(); }

    /// streambufからの1文字入力を処理し、ログにも残す。
    int_type underflow() {
        return write_log1(buf->sgetc());
    }

    /// streambufへの1文字出力を処理し、ログにも残す。
    int_type overflow(int_type c) {
        return write_log1(buf->sputc(traits_type::to_char_type(c)));
    }

    /// streambufからの複数文字入力を処理し、ログにも残す。
    std::streamsize xsgetn(char *ptr, std::streamsize count) {
        std::streamsize ret = buf->sgetn(ptr, count);
        write_logn(ptr, count);
        return ret;
    }

    /// streambufへの複数文字出力を処理し、ログにも残す。
    std::streamsize xsputn(const char *ptr, std::streamsize count) {
        write_logn(ptr, count);
        return buf->sputn(ptr, count);
    }
    
private:
    void flush_log() {
        auto line = logbuffer.str();
        logbuffer.str(""); // バッファのクリア

        // 改行を出力するときは空行でも出力する。
        log->sputn(prefix, 3);
        log->sputn(line.c_str(), line.size());
        log->sputc('\n');
        log->pubsync(); // 最後にflushしておく
    }

    /// 1文字用のログ出力
    /// 各行の先頭にprefixを出力し、エンジンにとっての入力か出力かをログ上で分かるようにする。
    int_type write_log1(int_type c) {
        // EOFなら何もしない。
        if (c == traits_type::eof()) {
            return c;
        }

        // 改行文字が来たら1行分のデータをまとめて出力する。
        if (c == '\n') {
            flush_log();
        }
        else {
            logbuffer << traits_type::to_char_type(c);
        }

        return c;
    }

    /// 複数文字のログ出力
    void write_logn(const char *ptr, std::streamsize count) {
        std::streamsize curr, prev=0;

        for (curr = 0; curr < count; ++curr) {
            if (ptr[curr] == '\n') {
                // 1度バッファに書き出してから1行分のログを出力
                // 直接ログに書き出すより遅くなるが、ソースが簡単になるのでこのようにする。
                logbuffer.write(&ptr[prev], curr - prev);
                flush_log();
                prev = curr + 1;
            }
        }

        // 改行を含まない領域をまとめてバッファに出力
        if (prev < count) logbuffer.write(&ptr[prev], count - prev);
    }

private:
    std::streambuf *buf; ///< コマンドの入出先
    std::streambuf *log; ///< ログの出力先
    std::stringstream logbuffer; ///< ログ出力用のバッファ。1行分保存する
    const char *prefix; ///< 出力時は'<< '、入力時は'>> 'をログの行頭に出力する
};


struct GodwhaleIO
{
    // ログ出力先にはIN/OUTともにcoutを設定します。
    GodwhaleIO()
        : cinbuf(nullptr), coutbuf(nullptr)
        , in(&socketbuf, std::cout.rdbuf(), false)
        , out(&socketbuf, std::cout.rdbuf(), true) {}
    ~GodwhaleIO() { close(); }

    void start(const std::string &host, const std::string &port) {
        std::cout << "connect to " << host << ":" << port << std::endl;

        // 接続できるまでループを回す
        while (socketbuf.connect(host, port) == nullptr) {
            std::cout << "retry..." << std::endl;

            std::this_thread::sleep_for(std::chrono::seconds(10));
        }

        std::cout << "succeeded to connect" << std::endl;
        cinbuf = std::cin.rdbuf(&in);
        coutbuf = std::cout.rdbuf(&out);
    }

    void close() {
        if (socketbuf.is_open()) {
            if (coutbuf != nullptr) {
                std::cout.rdbuf(coutbuf);
                coutbuf = nullptr;
            }
            if (cinbuf != nullptr) {
                std::cin.rdbuf(cinbuf);
                cinbuf = nullptr;
            }
            socketbuf.close();
            std::cout << "connection closed" << std::endl;
        }
    }

private:
    boost::asio::basic_socket_streambuf<boost::asio::ip::tcp> socketbuf;
    std::streambuf *cinbuf, *coutbuf;
    Tee in, out; // 標準入力とファイル、標準出力とファイルのひも付け
};


static std::unique_ptr<GodwhaleIO> io;

void start_godwhale_io(const std::string &host, const std::string &port)
{
    io.reset(new GodwhaleIO);
    io->start(host, port);
}

void close_godwhale_io()
{
    if (io != nullptr) {
        io->close();
        io = nullptr;
    }
}

#endif
