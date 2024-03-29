# Copyright 2023 Intrinsic Innovation LLC

# Common code for assets (e.g., skills, resources).

load("//bazel:go_macros.bzl", "go_library")
load("@rules_python//python:defs.bzl", "py_library")

package(default_visibility = ["//visibility:public"])

go_library(
    name = "clientutils",
    srcs = ["clientutils.go"],
    deps = [
        ":cmdutils",
        "//intrinsic/tools/inctl/auth",
        "@com_github_pkg_errors//:go_default_library",
        "@com_github_spf13_cobra//:go_default_library",
        "@org_golang_google_grpc//:go_default_library",
        "@org_golang_google_grpc//credentials:go_default_library",
        "@org_golang_google_grpc//credentials/insecure:go_default_library",
    ],
)

go_library(
    name = "cmdutils",
    srcs = ["cmdutils.go"],
    deps = [
        ":imageutils",
        "//intrinsic/tools/inctl/util:orgutil",
        "@com_github_pkg_errors//:go_default_library",
        "@com_github_spf13_cobra//:go_default_library",
        "@com_github_spf13_viper//:go_default_library",
    ],
)

cc_library(
    name = "id_utils",
    srcs = ["id_utils.cc"],
    hdrs = ["id_utils.h"],
    deps = [
        "//intrinsic/assets/proto:id_cc_proto",
        "//intrinsic/util/status:annotate",
        "//intrinsic/util/status:status_macros",
        "@com_github_google_re2//:re2",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/status:statusor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

py_library(
    name = "id_utils_py",
    srcs = ["id_utils.py"],
    deps = ["//intrinsic/assets/proto:id_py_pb2"],
)

go_library(
    name = "idutils",
    srcs = ["idutils.go"],
    deps = [
        "//intrinsic/assets/proto:id_go_proto",
        "@org_golang_x_exp//slices",
    ],
)

go_library(
    name = "metadatafieldlimits",
    srcs = ["metadata_field_limits.go"],
)

go_library(
    name = "imagetransfer",
    srcs = ["imagetransfer.go"],
    deps = [
        "@com_github_cenkalti_backoff_v4//:go_default_library",
        "@com_github_golang_glog//:go_default_library",
        "@com_github_google_go_containerregistry//pkg/name:go_default_library",
        "@com_github_google_go_containerregistry//pkg/v1:go_default_library",
        "@com_github_google_go_containerregistry//pkg/v1/remote:go_default_library",
        "@com_github_google_go_containerregistry//pkg/v1/remote/transport:go_default_library",
        "@com_github_pkg_errors//:go_default_library",
    ],
)

go_library(
    name = "imageutils",
    srcs = ["imageutils.go"],
    deps = [
        ":imagetransfer",
        "//intrinsic/assets/proto:id_go_proto",
        "//intrinsic/kubernetes/workcell_spec:imagetags",
        "//intrinsic/kubernetes/workcell_spec/proto:image_go_proto",
        "//intrinsic/kubernetes/workcell_spec/proto:installer_go_grpc_proto",
        "//intrinsic/util/proto:protoio",
        "@com_github_google_go_containerregistry//pkg/name:go_default_library",
        "@com_github_google_go_containerregistry//pkg/v1:go_default_library",
        "@com_github_google_go_containerregistry//pkg/v1/tarball:go_default_library",
        "@com_github_pkg_errors//:go_default_library",
        "@io_opencensus_go//trace:go_default_library",
        "@org_golang_google_grpc//:go_default_library",
        "@org_golang_google_grpc//codes:go_default_library",
        "@org_golang_google_grpc//status:go_default_library",
        "@org_golang_google_protobuf//proto",
    ],
)
