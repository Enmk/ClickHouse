import uuid
import os
import textwrap
import time
import xml.etree.ElementTree as xmltree

from contextlib import contextmanager
from collections import namedtuple

import testflows.settings as settings
from testflows.asserts import values, error, snapshot

from testflows.core import *

xml_with_utf8 = '<?xml version="1.0" encoding="utf-8"?>\n'
Config = namedtuple("Config", "content path name uid preprocessed_name")

def getuid():
    return str(uuid.uuid1()).replace('-', '_')

def xml_append(root, tag, text):
    element = xmltree.Element(tag)
    element.text = text
    root.append(element)
    return element

def xml_indent(elem, level=0, by="  "):
    i = "\n" + level * by
    if len(elem):
        if not elem.text or not elem.text.strip():
            elem.text = i + by
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
        for elem in elem:
            xml_indent(elem, level + 1)
        if not elem.tail or not elem.tail.strip():
            elem.tail = i
    else:
        if level and (not elem.tail or not elem.tail.strip()):
            elem.tail = i

def create_secret_xml_config(entries, inaccessible_secret=False, config_d_dir="/etc/clickhouse-server/config.d",
        config_file="secrets.xml"):
    """Create XML configuration file from a dictionary.
    """
    uid = getuid()
    path = os.path.join(config_d_dir, config_file)
    name = config_file

    if inaccessible_secret:
        root = xmltree.fromstring("<yandex><secrets>inaccessible_password</secrets></yandex>")
    else:
        root = xmltree.fromstring("<yandex><secrets></secrets></yandex>")

    xml_secrets = root.find("secrets")
    xml_secrets.append(xmltree.Comment(text=f"config uid: {uid}"))

    def create_xml_tree(entries, root):
        for k,v in entries.items():
            if type(v) is dict:
                xml_element = xmltree.Element(k)
                create_xml_tree(v, xml_element)
                root.append(xml_element)
            elif type(v) in (list, tuple):
                xml_element = xmltree.Element(k)
                for e in v:
                    create_xml_tree(e, xml_element)
                root.append(xml_element)
            else:
                xml_append(root, k, v)

    create_xml_tree(entries, xml_secrets)
    xml_indent(root)
    content = xml_with_utf8 + str(xmltree.tostring(root, short_empty_elements=False, encoding="utf-8"), "utf-8")

    return Config(content, path, name, uid, "config.xml")

@TestStep(Given)
def secrets_config(self, entries, inaccessible_secret=False, config_d_dir="/etc/clickhouse-server/config.d",
        config_file='secrets.xml', timeout=60, restart=True, config=None):
    """Add secrets config.
    """
    if config is None:
        config = create_secret_xml_config(entries=entries, inaccessible_secret=inaccessible_secret, config_d_dir=config_d_dir, config_file=config_file)
    return add_config(config, restart=restart)

def add_config(config, timeout=300, restart=False, modify=False):
    """Add dynamic configuration file to ClickHouse.

    :param node: node
    :param config: configuration file description
    :param timeout: timeout, default: 20 sec
    """
    node = current().context.node
    cluster = current().context.cluster

    def check_preprocessed_config_is_updated(after_removal=False):
        """Check that preprocessed config is updated.
        """
        started = time.time()
        command = f"cat /var/lib/clickhouse/preprocessed_configs/_{config.preprocessed_name} | grep {config.uid}{' > /dev/null' if not settings.debug else ''}"

        while time.time() - started < timeout:
            exitcode = node.command(command, steps=False).exitcode
            if after_removal:
                if exitcode == 1:
                    break
            else:
                if exitcode == 0:
                    break
            time.sleep(1)

        if settings.debug:
            node.command(f"cat /var/lib/clickhouse/preprocessed_configs/_{config.preprocessed_name}")

        if after_removal:
            assert exitcode == 1, error()
        else:
            assert exitcode == 0, error()

    def wait_for_config_to_be_loaded():
        """Wait for config to be loaded.
        """
        if restart:
            with When("I close terminal to the node to be restarted"):
                bash.close()

            with And("I stop ClickHouse to apply the config changes"):
                node.stop(safe=False)

            with And("I get the current log size"):
                cmd = node.cluster.command(None,
                    f"stat --format=%s {cluster.environ['CLICKHOUSE_TESTS_DIR']}/_instances/{node.name}/logs/clickhouse-server.log")
                logsize = cmd.output.split(" ")[0].strip()

            with And("I start ClickHouse back up"):
                node.start()

            with Then("I tail the log file from using previous log size as the offset"):
                bash.prompt = bash.__class__.prompt
                bash.open()
                bash.send(f"tail -c +{logsize} -f /var/log/clickhouse-server/clickhouse-server.log")

        with Then("I wait for config reload message in the log file"):
            if restart:
                bash.expect(
                    f"ConfigReloader: Loaded config '/etc/clickhouse-server/config.xml', performed update on configuration",
                    timeout=timeout)
            else:
                bash.expect(
                    f"ConfigReloader: Loaded config '/etc/clickhouse-server/{config.preprocessed_name}', performed update on configuration",
                    timeout=timeout)

    try:
        with Given(f"{config.name}"):
            if settings.debug:
                with When("I output the content of the config"):
                    debug(config.content)

            with node.cluster.shell(node.name) as bash:
                bash.expect(bash.prompt)
                bash.send("tail -n 0 -f /var/log/clickhouse-server/clickhouse-server.log")

                with When("I add the config", description=config.path):
                    command = f"cat <<HEREDOC > {config.path}\n{config.content}\nHEREDOC"
                    node.command(command, steps=False, exitcode=0)

                with Then(f"{config.preprocessed_name} should be updated", description=f"timeout {timeout}"):
                    check_preprocessed_config_is_updated()

                with And("I wait for config to be reloaded"):
                    wait_for_config_to_be_loaded()
        yield
    finally:
        if not modify:
            with Finally(f"I remove {config.name}"):
                with node.cluster.shell(node.name) as bash:
                    bash.expect(bash.prompt)
                    bash.send("tail -n 0 -f /var/log/clickhouse-server/clickhouse-server.log")

                    with By("removing the config file", description=config.path):
                        node.command(f"rm -rf {config.path}", exitcode=0)

                    with Then(f"{config.preprocessed_name} should be updated", description=f"timeout {timeout}"):
                        check_preprocessed_config_is_updated(after_removal=True)

                    with And("I wait for config to be reloaded"):
                        wait_for_config_to_be_loaded()

@TestStep(Given)
def secrets_invalid_config(self, entries, config_d_dir="/etc/clickhouse-server/config.d",
        config_file='secrets.xml', timeout=60, restart=True, config=None):
    """Add invalid secrets config.
    """
    if config is None:
        config = create_secret_xml_config(entries=entries, config_d_dir=config_d_dir, config_file=config_file)
    return invalid_server_config(config=config)

def invalid_server_config(config, tail=30, timeout=300):
    """Check that ClickHouse errors when trying to load invalid LDAP servers configuration file.
    """
    node = current().context.node
    message = "Exception: Failed to merge config with '/etc/clickhouse-server/config.d/secrets.xml'"

    try:
        node.command("echo -e \"%s\" > /var/log/clickhouse-server/clickhouse-server.err.log" % ("-\\n" * tail))

        with When("I add the config", description=config.path):
            command = f"cat <<HEREDOC > {config.path}\n{config.content}\nHEREDOC"
            node.command(command, steps=False, exitcode=0)

        with Then("server shall fail to merge the new config"):
            started = time.time()
            command = f"tail -n {tail} /var/log/clickhouse-server/clickhouse-server.err.log | grep \"{message}\""
            while time.time() - started < timeout:
                exitcode = node.command(command, steps=False).exitcode
                if exitcode == 0:
                    break
                time.sleep(1)
            assert exitcode == 0, error()

    finally:
        with Finally(f"I remove {config.name}"):
            with By("removing the config file", description=config.path):
                node.command(f"rm -rf {config.path}", exitcode=0)

@TestStep(Given)
def secret_leak_check(self, secret_message, query_ids):
    """Check for leaks in query log, server log, and system processes.
    """
    try:
        with Given("I prepare the server log"):
            check_leaks_in_server_log(secret_message=secret_message)

        yield

    finally:

        for query_id in query_ids:

            with Finally("Check the system processes"):
                check_leaks_in_system_processes(secret_message=secret_message, query_id=query_id)

            with And("Check the query log"):
                check_leaks_in_query_log(secret_message=secret_message, query_id=query_id)

@TestStep(Given)
def check_leaks_in_server_log(self, secret_message, node=None,
        clickhouse_server_log="/var/log/clickhouse-server/clickhouse-server.log"):
    """Make sure secrets didn't leak into the server log.
    """

    if node is None:
        node = self.context.node

    with By("getting current log size"):
        cmd =  node.command(f"stat --format=%s {clickhouse_server_log}")
        start_logsize = cmd.output.split(" ")[0].strip()

    try:
        yield

    finally:

        with Finally("getting current log size at the end of the test"):
           cmd =  node.command(f"stat --format=%s {clickhouse_server_log}")
           end_logsize = cmd.output.split(" ")[0].strip()

        with Then("dumping clickhouse-server.log for this test"):
            exitcode = node.command(f"tail -c +{start_logsize} {clickhouse_server_log}"
                f" | head -c {int(end_logsize) - int(start_logsize)}"
                f" | grep {secret_message}").exitcode
            assert exitcode == 1, error()

@TestStep(Finally)
def check_leaks_in_query_log(self, secret_message, query_id, node=None):
    """Make sure secrets didn't leak into the query log.
    """

    if node is None:
        node = self.context.node

    with Finally("I check the query log for the secret value"):
        exitcode = node.command(f"clickhouse-client -q\"SELECT * FROM system.query_log WHERE query_id='{query_id}' FORMAT JSON\" | grep {secret_message}").exitcode
        assert exitcode == 1, error()

@TestStep(Finally)
def check_leaks_in_system_processes(self, secret_message, query_id, node=None):
    """Make sure secrets didn't leak into the system processes.
    """

    if node is None:
        node = self.context.node

    with Finally("I check the system processes for the secret value"):
        exitcode = node.command(f"clickhouse-client -q\"SELECT * FROM system.system_processes WHERE query_id='{query_id}' FORMAT JSON\" | grep {secret_message}").exitcode
        assert exitcode == 1, error()

@TestStep(Given)
def table(self, name, node, mysql_node, user_name, user_password):
    """Create a table and user in MySQL.
    """
    try:

        with Given("table in MySQL"):
            sql = f"""
                CREATE TABLE {name}(
                    id INT NOT NULL AUTO_INCREMENT,
                    x INT,
                    PRIMARY KEY ( id )
                );
                """
            with When("I drop the table if exists"):
                mysql_node.command(f"MYSQL_PWD=password mysql -D db -u user -e \"DROP TABLE IF EXISTS {name};\"", exitcode=0)

            with And("I create a table"):
                mysql_node.command(f"MYSQL_PWD=password mysql -D db -u user <<'EOF'{textwrap.dedent(sql)}\nEOF", exitcode=0)

            with And("I create a user"):
                mysql_node.command(f"MYSQL_PWD=password mysql -D db -u root -e \"CREATE USER {user_name} IDENTIFIED BY '{user_password}'\"")

            with And("I grant priviliges to the user"):
                mysql_node.command(f"MYSQL_PWD=password mysql -D db -u root -e \"GRANT ALL PRIVILEGES ON db.* TO '{user_name}'\"")

        yield

    finally:

        with Finally("I drop a table in MySQL", flags=TE):
            mysql_node.command(f"MYSQL_PWD=password mysql -D db -u user -e \"DROP TABLE IF EXISTS {name};\"", exitcode=0)

        with And("I drop the user", flags=TE):
            mysql_node.command(f"MYSQL_PWD=password mysql -D db -u root -e \"DROP USER IF EXISTS {name};\"")
