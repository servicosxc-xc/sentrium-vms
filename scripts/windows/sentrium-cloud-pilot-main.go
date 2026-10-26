// SENTRIUM F005 — experimental operator-owned Cloud bridge runner.
//
// Build from the pinned MIT-licensed upstream dh-fwd module (OUTSIDE this
// repository). SENTRIUM does not ship or auto-install dh-fwd or SIM Next.
// A local proof only; not a native Cloud provider for production.
//
// Example (run from the upstream dh-fwd checkout, Go 1.25+):
//   go build -o C:/Temp/sentrium-cloud-pilot.exe C:/.../sentrium-cloud-pilot-main.go
//
// The wrapper accepts the Cloud ID and optional device authentication from
// inherited environment variables, avoiding password exposure in the OS-level
// process launch arguments. Upstream core uses os.Args internally; the wrapper
// therefore populates those after process start. Same-user processes with
// debug privileges may still read the environment/process memory.
package main

import (
	"fmt"
	"os"
	"strings"

	"dh-fwd/core"
)

func main() {
	const idName = "SENTRIUM_PILOT_CLOUD_ID"
	const userName = "SENTRIUM_PILOT_USER"
	const passwordName = "SENTRIUM_PILOT_PASSWORD"

	cloudID := strings.TrimSpace(os.Getenv(idName))
	username := os.Getenv(userName)
	password := os.Getenv(passwordName)

	// Remove inherited variables from the Go process environment; the values
	// will still exist in Go memory until reclaimed. No persistent storage.
	_ = os.Unsetenv(idName)
	_ = os.Unsetenv(userName)
	_ = os.Unsetenv(passwordName)

	if cloudID == "" || len(cloudID) > 128 ||
		strings.ContainsAny(cloudID, " \t\r\n") {
		fmt.Fprintln(os.Stderr, "SENTRIUM pilot: Cloud ID is absent or invalid")
		os.Exit(2)
	}
	if (username == "") != (password == "") {
		fmt.Fprintln(os.Stderr, "SENTRIUM pilot: specify both user and password for authenticated Cloud")
		os.Exit(2)
	}
	for _, arg := range os.Args[1:] {
		// Reject potentially conflicting CLI credential/serial options.
		// Only the caller-provided environment controls identity and auth.
		if arg == "-c" || arg == "--creds" ||
			arg == "-u" || arg == "--username" ||
			arg == "-P" || arg == "--password" ||
			arg == "-t" || arg == "--type" {
			fmt.Fprintln(os.Stderr, "SENTRIUM pilot: credential flags are not permitted")
			os.Exit(2)
		}
	}

	if username != "" {
		os.Args = append(os.Args, "-t", "1", "-u", username, "-P", password)
	}
	os.Args = append(os.Args, cloudID)
	core.Execute()
}
