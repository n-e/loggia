# Blah

Blah is a fast web log analyzer that runs in the terminal. It requires no setup or databases and is great for quickly getting useful information out of HTTP logs.

## Quick Start

    htdrill [ options ... ] rows [ cols [ filter ] ]

    rows: field to use as row headers (do XXX for details).
    cols: same as rows but for columns. If left blank show totals for each row.
    filter: field=value. field<>value

## Installation

Why :
 - Quick and easy to install
 - Deep insights quickly
 - Very fast ( O(n) on XXX, O(n.log n)

 - written in C with no dependencies (OSX, arch, ubuntu)