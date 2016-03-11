#!/bin/sh

exec `dirname $0`/perflock -s "$@"
