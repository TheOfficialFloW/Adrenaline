#!/bin/bash

store_git_rev() {
    repo_dir_name="$(basename "$(pwd)")"
    branch="$(git symbolic-ref --short -q HEAD || echo "detached")"
    commit="$(git rev-parse HEAD)"
    echo "${branch} ${commit}" > "$1/${repo_dir_name}_rev.txt"
}