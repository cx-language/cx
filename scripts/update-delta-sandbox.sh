#!/bin/sh

if [ $TRAVIS_REPO_SLUG != "delta-lang/delta" ] || [ $TRAVIS_BRANCH != "master" ]; then
    exit;
fi

git clone https://github.com/delta-lang/delta-sandbox.git
cd delta-sandbox

DELTA_VERSION=$TRAVIS_COMMIT
echo $DELTA_VERSION > server/.delta-version
git add server/.delta-version
git commit --message "Update Delta version to $DELTA_VERSION" --author "Delta CI <>"

git remote set-url origin https://${GITHUB_TOKEN}@github.com/delta-lang/delta-sandbox.git > /dev/null 2>&1
git push --quiet
