#!/usr/bin/env python3
import os
import sys

from testflows.core import *

append_path(sys.path, "..")

from helpers.cluster import Cluster
from helpers.argparser import argparser
from secrets_management.requirements import SRS_021_ClickHouse_Secrets_Management

xfails = {
}

xflags = {
}

@TestModule
@ArgumentParser(argparser)
@XFails(xfails)
@XFlags(xflags)
@Name("Secrets Management")
@Specifications(
    SRS_021_ClickHouse_Secrets_Management
)
def regression(self, local, clickhouse_binary_path, stress=None, parallel=None):
    """Secrets Management regression.
    """
    top().terminating = False

    nodes = {
        "clickhouse":
            ("clickhouse1", "clickhouse2", "clickhouse3")
    }
    with Cluster(local, clickhouse_binary_path, nodes=nodes,
            docker_compose_project_dir=os.path.join(current_dir(), "secrets_management_env")) as cluster:
        self.context.cluster = cluster
        self.context.stress = stress

        if parallel is not None:
            self.context.parallel = parallel

        Feature(run=load("secrets_management.tests.feature", "feature"))

if main():
    regression()
