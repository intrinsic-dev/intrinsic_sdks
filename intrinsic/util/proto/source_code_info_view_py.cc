// Copyright 2023 Intrinsic Innovation LLC
// Intrinsic Proprietary and Confidential
// Provided subject to written agreement between the parties.

#include <pybind11/pybind11.h>

#include <string>

#include "absl/container/flat_hash_map.h"
#include "absl/status/status.h"
#include "absl/status/statusor.h"
#include "absl/strings/string_view.h"
#include "google/protobuf/descriptor.pb.h"
#include "google/protobuf/map.h"
#include "intrinsic/util/proto/source_code_info_view.h"
#include "pybind11_abseil/absl_casters.h"
#include "pybind11_abseil/status_casters.h"
#include "pybind11_protobuf/wrapped_proto_caster.h"

namespace intrinsic {

class SourceCodeInfoViewPython {
  // Wrapper for the C++ implementation of SourceCodeInfoView, with some
  // functions containing minor differences.
 public:
  absl::Status Init(
      const google::protobuf::FileDescriptorSet& file_descriptor_set) {
    return source_code_info_view_.Init(file_descriptor_set);
  }

  absl::StatusOr<std::string> GetLeadingCommentsByFieldName(
      absl::string_view field_name) {
    return source_code_info_view_.GetLeadingCommentsByFieldName(field_name);
  }

  // pybind has an issue with automatically converting google::protobuf::Map,
  // this works around it by copying the output of the existing
  // function to an absl::flat_hash_map
  absl::flat_hash_map<std::string, std::string> GetNestedFieldCommentMap(
      absl::string_view message_name) {
    auto status_or_map =
        source_code_info_view_.GetNestedFieldCommentMap(message_name);
    if (!status_or_map.ok()) {
      return absl::flat_hash_map<std::string, std::string>();
    }
    return absl::flat_hash_map<std::string, std::string>(
        status_or_map.value().begin(), status_or_map.value().end());
  }

 private:
  SourceCodeInfoView source_code_info_view_;
};

PYBIND11_MODULE(source_code_info_view_py, m) {
  pybind11::google::ImportStatusModule();
  pybind11_protobuf::ImportWrappedProtoCasters();

  using pybind11_protobuf::WithWrappedProtos;

  pybind11::class_<SourceCodeInfoViewPython>(m, "SourceCodeInfoView")
      .def(pybind11::init<>())
      .def("Init", WithWrappedProtos(&SourceCodeInfoViewPython::Init),
           pybind11::arg("file_descriptor_set"))
      .def("GetLeadingCommentsByFieldName",
           WithWrappedProtos(
               &SourceCodeInfoViewPython::GetLeadingCommentsByFieldName),
           pybind11::arg("field_name"))
      .def("GetNestedFieldCommentMap",
           WithWrappedProtos(
               &SourceCodeInfoViewPython::GetNestedFieldCommentMap),
           pybind11::arg("message_name"));
}

}  // namespace intrinsic
