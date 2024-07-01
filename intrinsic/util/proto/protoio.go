// Copyright 2023 Intrinsic Innovation LLC

// Package protoio defines common functions for working with text and binary
// proto files.
package protoio

import (
	"fmt"
	"os"

	"github.com/protocolbuffers/txtpbfmt/parser"
	"google.golang.org/protobuf/encoding/prototext"
	"google.golang.org/protobuf/proto"
	"google.golang.org/protobuf/reflect/protoregistry"
)

// TextReadOption provides a way to update UnmarshalOptions used in
// ReadTextProto
type TextReadOption = func(*prototext.UnmarshalOptions)

// BinaryReadOption provides a way to update UnmarshalOptions used in
// ReadBinaryProto
type BinaryReadOption = func(*proto.UnmarshalOptions)

// BinaryWriteOption  provides a way to update MarshalOptions used in
// WriteBinaryProto
type BinaryWriteOption = func(*proto.MarshalOptions)

// Resolver is the interface required to be a resolver for proto or prototext.
type Resolver = interface {
	protoregistry.ExtensionTypeResolver
	protoregistry.MessageTypeResolver
}

// WithResolver is a helper to create a TextReadOption for use with
// ReadTextProto.  Often protoregistry.Type is used as the resolver.
func WithResolver(resolver Resolver) TextReadOption {
	return func(options *prototext.UnmarshalOptions) {
		options.Resolver = resolver
	}
}

// WithMerge is a helper to create a BinaryReadOption for use with
// ReadBinaryProto.  It sets the Merge field of proto.UnmarshalOptions to the
// provided value.
func WithMerge(value bool) BinaryReadOption {
	return func(options *proto.UnmarshalOptions) {
		options.Merge = value
	}
}

// WithDeterministic is a helper to create a BinaryWriteOption for use with
// WriteBinaryProto.  It sets the Deterministic field of proto.MarshalOptions
// to the provided value.
func WithDeterministic(value bool) BinaryWriteOption {
	return func(options *proto.MarshalOptions) {
		options.Deterministic = value
	}
}

// ReadTextProto reads a proto message encoded as pbtxt from a file.
func ReadTextProto(path string, p proto.Message, opts ...TextReadOption) error {
	b, err := os.ReadFile(path)
	if err != nil {
		return fmt.Errorf("reading %q failed: %w", path, err)
	}

	options := new(prototext.UnmarshalOptions)
	for _, opt := range opts {
		opt(options)
	}
	if err := options.Unmarshal(b, p); err != nil {
		return fmt.Errorf("parsing proto file %q failed failed: %w", path, err)
	}
	return nil
}

// ReadBinaryProto reads a binary encoded proto message from a file.
func ReadBinaryProto(path string, p proto.Message, opts ...BinaryReadOption) error {
	b, err := os.ReadFile(path)
	if err != nil {
		return fmt.Errorf("failed to read %q: %w", path, err)
	}

	options := new(proto.UnmarshalOptions)
	for _, opt := range opts {
		opt(options)
	}
	if err := options.Unmarshal(b, p); err != nil {
		return fmt.Errorf("parsing the message from %q failed: %w", path, err)
	}
	return nil
}

// WriteBinaryProto writes a binary encoded proto message to a file.
func WriteBinaryProto(path string, p proto.Message, opts ...BinaryWriteOption) error {
	options := new(proto.MarshalOptions)
	for _, opt := range opts {
		opt(options)
	}
	b, err := options.Marshal(p)
	if err != nil {
		return fmt.Errorf("failed to serialize %q: %w", path, err)
	}

	if err := os.WriteFile(path, b, 0644); err != nil {
		return fmt.Errorf("failed to write %q: %w", path, err)
	}
	return nil
}

// WriteStableTextProto writes out a textproto that has been formatted by
// standard formatting txtpbfmt, and thus is stable to use in build rules.
func WriteStableTextProto(path string, p proto.Message) error {
	b, err := prototext.Marshal(p)
	if err != nil {
		return fmt.Errorf("failed to serialize: %v", err)
	}
	b, err = parser.FormatWithConfig(b, parser.Config{
		ExpandAllChildren: true,
		SkipAllColons:     true,
	})
	if err != nil {
		return fmt.Errorf("failed to format: %v", err)
	}

	if err := os.WriteFile(path, b, 0644); err != nil {
		return fmt.Errorf("failed to write file: %v", err)
	}
	return nil
}
