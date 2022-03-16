// Copyright 2018 Josh Bialkowski <josh.bialkowski@gmail.com>

#include "argue/glog.h"

#include <glog/logging.h>

#include "argue/argue.h"

namespace argue {

// Add glog options (normally exposed through gflags) to the parser
void add_glog_options(Parser* parser) {
  {
    auto opts = parser->add_argument("--log-to-stderr", &FLAGS_logtostderr,
                                     {.action = "store_true"});
    opts.help = "Set whether log messages go to stderr instead of logfiles";
  }
  {
    auto opts =
        parser->add_argument("--also-log-to-stderr", &FLAGS_alsologtostderr,
                             {.action = "store_true"});
    opts.help =
        "Set whether log messages go to stderr in addition to logfiles.";
  }
  {
    auto opts =
        parser->add_argument("--color-log-to-stderr", &FLAGS_colorlogtostderr,
                             {.action = "store_true"});
    opts.help =
        "Set color messages logged to stderr (if supported by terminal).";
  }
  {
    auto opts =
        parser->add_argument("--stderr-threshold", &FLAGS_stderrthreshold);
    opts.help =
        "Copy log messages at or above this level to stderr in addition to "
        "logfiles. The numbers of severity levels INFO, WARNING, ERROR, and "
        "FATAL are 0, 1, 2, and 3, respectively.";
  }
  {
    auto opts = parser->add_argument("--log-prefix", &FLAGS_log_prefix);
    opts.help =
        "Set whether the log prefix should be prepended "
        "to each line of output.";
  }
  {
    auto opts = parser->add_argument("--min-log-level", &FLAGS_minloglevel);
    opts.help =
        "Log messages at or above this level. Again, the numbers of "
        "severity levels INFO, WARNING, ERROR, and FATAL are 0, 1, 2, "
        "and 3, respectively.";
  }
  {
    auto opts = parser->add_argument("--log-dir", &FLAGS_log_dir);
    opts.help =
        "If specified, logfiles are written into this directory "
        "instead of the default loggind directory.";
  }
  {
    auto opts = parser->add_argument("-v", "--verbose", &FLAGS_v);
    opts.help =
        "Show all VLOG(m) messages for m less or equal the value of "
        "this flag. Overridable by --vmodule. See the section about "
        "verbose logging for more detail.";
  }
}

}  // namespace argue
