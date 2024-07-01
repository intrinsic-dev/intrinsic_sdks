// Copyright 2023 Intrinsic Innovation LLC

// Package templateutil contains helpers for working with templates in the inctl tool.
package templateutil

import (
	"errors"
	"fmt"
	"os"
	"path/filepath"
	"text/template"
)

type any = interface{}

// CheckFilesDoNotExist returns an error if any of the given files does
// already exist.
func CheckFilesDoNotExist(paths []string) error {
	for _, path := range paths {
		if err := CheckFileDoesNotExist(path); err != nil {
			return err
		}
	}
	return nil
}

// CheckFileDoesNotExist returns an error if the given file does
// already exist.
func CheckFileDoesNotExist(path string) error {
	if _, err := os.Stat(path); err == nil {
		return fmt.Errorf("file %q: %w", path, os.ErrExist)
	}
	return nil
}

// CreateFileOptions defines optional settings for file creation.
type CreateFileOptions struct {
	Override bool
}

// CreateNewFileFromTemplate creates a new file with the given path from the
// template with the given name in the given template set. Returns an error
// if the file already exist.
func CreateNewFileFromTemplate(path string, templateName string, data any, templateSet *template.Template, createFileOptions CreateFileOptions) error {
	// Error out if we would be overwriting an existing file unless overwriting is intended.
	if _, err := os.Stat(path); err == nil && !createFileOptions.Override {
		return fmt.Errorf("file %s cannot be created since it already exists: %w", path, os.ErrExist)
	}

	if err := os.MkdirAll(filepath.Dir(path), 0770 /*rwxrwx---*/); err != nil {
		return fmt.Errorf("creating directory %s: %w", filepath.Dir(path), err)
	}
	file, err := os.OpenFile(path, os.O_RDWR|os.O_CREATE, 0660 /*rw-rw----*/)
	if err != nil {
		return fmt.Errorf("creating file %s: %w", path, err)
	}
	defer file.Close()

	err = templateSet.ExecuteTemplate(file, templateName, data)
	if err != nil {
		return fmt.Errorf("executing template %q: %w", templateName, err)
	}

	return nil
}

// AppendToExistingFileFromTemplate appends to the file with the given path the
// contents of the template with the given name in the given template set.
// Returns an error if the file does not exist.
func AppendToExistingFileFromTemplate(path string, templateName string, data any, templateSet *template.Template) error {
	if _, err := os.Stat(path); errors.Is(err, os.ErrNotExist) {
		return fmt.Errorf("appending to non-existing file %s: %w", path, err)
	}

	file, err := os.OpenFile(path, os.O_APPEND|os.O_WRONLY, 0644)
	if err != nil {
		return fmt.Errorf("opening existing file %s for writing: %w", path, err)
	}
	defer file.Close()

	err = templateSet.ExecuteTemplate(file, templateName, data)
	if err != nil {
		return fmt.Errorf("executing template \"%s\": %w", templateName, err)
	}

	return nil
}
