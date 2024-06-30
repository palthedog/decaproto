package main

import (
	"bytes"
	"fmt"
	"os"
	"text/template"
)

func print(name string, tpl string, args map[string]string) string {
	buf := bytes.NewBuffer(nil)
	t, err := template.New(name).Parse(tpl)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error parsing template %s: %s\n", name, err)
		panic(err)
	}

	err = t.Option("missingkey=error").Execute(buf, args)
	if err != nil {
		fmt.Fprintf(os.Stderr, "Error executing template %s: %s\n%s\n", name, args, err)
		panic(err)
	}
	return buf.String()
}
