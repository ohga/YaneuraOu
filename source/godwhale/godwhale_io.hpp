
#ifndef GODWHALE_CHILD_GODWHALE_IO_HPP
#define GODWHALE_CHILD_GODWHALE_IO_HPP

#include <string>

#if defined(GODWHALE_CLUSTER_SLAVE)

// ----------------------
//  stream for godwhale
// ----------------------
extern void start_godwhale_io(const std::string &host, const std::string &port);
extern void close_godwhale_io();

#endif

#endif
