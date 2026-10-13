# DPP CA certificate bundle

`dpp-cacert.pem` is the Mozilla root CA bundle published by curl:
https://curl.se/docs/caextract.html

The bundle is distributed under the Mozilla Public License 2.0:
https://www.mozilla.org/MPL/2.0/

Builds with DPP enabled deploy this file beside the SerialPrograms executable
on Windows, Linux, and macOS (inside `SerialPrograms.app/Contents/MacOS` on macOS).
At startup, SerialPrograms sets SSL_CERT_FILE to that deployed bundle unless
the caller already configured SSL_CERT_FILE. Missing or unreadable bundles
are logged.

To refresh the bundle, download https://curl.se/ca/cacert.pem to a temporary
file, verify that it contains PEM certificates, then replace dpp-cacert.pem.
Rebuild to deploy the new copy and restart SerialPrograms to reload it.
