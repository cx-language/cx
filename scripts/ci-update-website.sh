#!/bin/sh

if [ "$TRAVIS_REPO_SLUG" != "delta-lang/delta" ] || [ "$TRAVIS_BRANCH" != "master" ]; then
    exit
fi

(
    cd docs || exit
    ./build-website.sh
)

git clone https://github.com/delta-lang/delta-lang.github.io.git
cd delta-lang.github.io || exit

DELTA_VERSION=$TRAVIS_COMMIT
echo "$DELTA_VERSION" > server/.delta-version

cp -a ../docs/build/. ./
git add .

PREVIOUS_COMMIT_AUTHOR=$(git log -1 --pretty=format:%an)
if [ "$PREVIOUS_COMMIT_AUTHOR" = "Delta CI" ]; then
    AMEND=--amend
fi

git commit --message "Update documentation and Delta version to $DELTA_VERSION" --author "Delta CI <>" $AMEND

git remote set-url origin "https://${GITHUB_TOKEN}@github.com/delta-lang/delta-lang.github.io.git" > /dev/null 2>&1
git push --quiet --force-with-lease
