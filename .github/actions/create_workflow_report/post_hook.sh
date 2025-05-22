#!/bin/bash
# This script is for generating preview reports when invoked as a post-hook from a praktika job
python3 -m venv .venv
source .venv/bin/activate
pip install clickhouse-driver==0.2.8 numpy==1.26.4 pandas==2.0.3 jinja2==3.1.5
python3 .github/actions/create_workflow_report/create_workflow_report.py --mark-preview --known-fails tests/broken_tests.json --cves --actions-run-url $GITHUB_SERVER_URL/$GITHUB_REPOSITORY/actions/runs/$GITHUB_RUN_ID
deactivate
rm -rf .venv
