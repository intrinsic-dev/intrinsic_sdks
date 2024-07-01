// Copyright 2023 Intrinsic Innovation LLC

#include "intrinsic/util/status/ret_check.h"

#include <cstddef>
#include <memory>
#include <ostream>
#include <sstream>
#include <string>

#include "absl/base/log_severity.h"
#include "absl/status/status.h"
#include "intrinsic/icon/release/source_location.h"
#include "intrinsic/util/status/status_builder.h"

namespace intrinsic {
namespace internal_status_macros_ret_check {

StatusBuilder RetCheckFailSlowPath(intrinsic::SourceLocation location) {
  return InternalErrorBuilder(location)
             .Log(absl::LogSeverity::kError)
             .EmitStackTrace()
         << "INTR_RET_CHECK failure (" << location.file_name() << ":"
         << location.line() << ") ";
}

StatusBuilder RetCheckFailSlowPath(intrinsic::SourceLocation location,
                                   std::string* condition) {
  std::unique_ptr<std::string> cleanup(condition);
  return RetCheckFailSlowPath(location) << *condition << " ";
}

StatusBuilder RetCheckFailSlowPath(intrinsic::SourceLocation location,
                                   const char* condition) {
  return RetCheckFailSlowPath(location) << condition << " ";
}

StatusBuilder RetCheckFailSlowPath(intrinsic::SourceLocation location,
                                   const char* condition,
                                   const absl::Status& status) {
  return RetCheckFailSlowPath(location)
         << condition << " returned " << status.ToString() << " ";
}

CheckOpMessageBuilder::CheckOpMessageBuilder(const char* exprtext)
    : stream_(new std::ostringstream) {
  *stream_ << exprtext << " (";
}

CheckOpMessageBuilder::~CheckOpMessageBuilder() { delete stream_; }

std::ostream* CheckOpMessageBuilder::ForVar2() {
  *stream_ << " vs. ";
  return stream_;
}

std::string* CheckOpMessageBuilder::NewString() {
  *stream_ << ")";
  return new std::string(stream_->str());
}

void MakeCheckOpValueString(std::ostream* os, char v) {
  if (v >= 32 && v <= 126) {
    (*os) << "'" << v << "'";
  } else {
    (*os) << "char value " << int{v};
  }
}

void MakeCheckOpValueString(std::ostream* os, signed char v) {
  if (v >= 32 && v <= 126) {
    (*os) << "'" << v << "'";
  } else {
    (*os) << "signed char value " << int{v};
  }
}

void MakeCheckOpValueString(std::ostream* os, unsigned char v) {
  if (v >= 32 && v <= 126) {
    (*os) << "'" << v << "'";
  } else {
    (*os) << "unsigned char value " << int{v};
  }
}

void MakeCheckOpValueString(std::ostream* os, std::nullptr_t) {
  (*os) << "nullptr";
}

void MakeCheckOpValueString(std::ostream* os, const char* v) {
  if (v == nullptr) {
    (*os) << "nullptr";
  } else {
    (*os) << v;
  }
}

void MakeCheckOpValueString(std::ostream* os, const signed char* v) {
  if (v == nullptr) {
    (*os) << "nullptr";
  } else {
    (*os) << v;
  }
}

void MakeCheckOpValueString(std::ostream* os, const unsigned char* v) {
  if (v == nullptr) {
    (*os) << "nullptr";
  } else {
    (*os) << v;
  }
}

void MakeCheckOpValueString(std::ostream* os, char* v) {
  if (v == nullptr) {
    (*os) << "nullptr";
  } else {
    (*os) << v;
  }
}

void MakeCheckOpValueString(std::ostream* os, signed char* v) {
  if (v == nullptr) {
    (*os) << "nullptr";
  } else {
    (*os) << v;
  }
}

void MakeCheckOpValueString(std::ostream* os, unsigned char* v) {
  if (v == nullptr) {
    (*os) << "nullptr";
  } else {
    (*os) << v;
  }
}

}  // namespace internal_status_macros_ret_check
}  // namespace intrinsic
