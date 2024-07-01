<!--
Copyright 2023 Intrinsic Innovation LLC
-->
# Intrinsic SDK

## Repository Deprecation Notice

This repository will be archived on July 1st, 2024.
From that date forward, any Intrinsic software updates to our SDK will only be accessible at our new repository [`intrinsic-dev/sdk`](https://github.com/intrinsic-dev/sdk). 

To continue accessing the latest updates to our SDK, you need to update your Bazel `WORKSPACE` file to the new repository.
Follow these steps:

1. Open your `WORKSPACE` file
1. Locate the `git_repository` rule for the Intrinsic SDK
1. Replace the old URL `https://github.com/intrinsic-dev/intrinsic_sdks.git` with the new one `https://github.com/intrinsic-dev/sdk.git` 

A correctly updated `WORKSPACE` file looks like this:

```
git_repository(
   name = "ai_intrinsic_sdks",
   remote = "https://github.com/intrinsic-dev/sdk.git",
   tag = "v1.8.20240603",
)
```

## About the Intrinsic SDK

The Intrinsic SDK is a collection of application programming interfaces
(APIs) and tools for software developers to write code that works with
[Flowstate](https://intrinsic.ai/flowstate), a web-based tool to build
robotic solutions from concept to deployment.

In addition to this
[SDK repository](https://github.com/intrinsic-dev/intrinsic_sdks), there is a
companion repository with
[examples](https://github.com/intrinsic-dev/sdk-examples) and a dev container
[project template](https://github.com/intrinsic-dev/project-template).

## Disclaimer

As Flowstate and the SDK are in beta, the contents of this repository are
subject to change.
Use of this repository requires participation in the beta for Intrinsic
Flowstate, which is accepting [applications](https://intrinsic.ai/beta).
Access to this repository is subject to the associated [LICENSE](LICENSE).
