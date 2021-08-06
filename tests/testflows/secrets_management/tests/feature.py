from testflows.core import *
from testflows.core.name import basename, parentname
from testflows._core.testtype import TestSubType

from common import *
from secrets_management.requirements import *

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_Select("1.0"),
)
def select(self, node=None):
    """Check that SELECT does not show the content of select.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_password'

    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'literal_password': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    with When('I use SELECT with a secret'):
        node.query("SELECT secret('literal_password')",
            message='DB::Exception: Serialization is not implemented', exitcode=48, query_id=query_id)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_Syntax("1.0"),
)
def multiple_inputs(self, node=None):
    """Check that secrets only work with one input.
    """
    if node is None:
        node = self.context.node

    query_id = getuid()

    with Given("I have a config with multiple subsections"):
        secrets_config(entries={'a':'sub_password_a','b':'sub_password_b'})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message='sub_password', query_ids=[query_id])

    with When('I use SELECT with multiple secret arguments'):
        node.query("SELECT secret('a','b')",
            message="Exception: Number of arguments for function secret doesn't match: passed 2", exitcode=42, query_id=query_id)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_UTF8Message("1.0")
)
def utf8_secret_message(self, node=None):
    """Check that secrets work with utf8 characters.
    """
    if node is None:
        node = self.context.node

    secret_message = '🔑'

    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'utf8_secret': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    with When('I use SELECT with a secret'):
        node.query("SELECT secret('utf8_secret')",
            message='DB::Exception: Serialization is not implemented', exitcode=48, query_id=query_id)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_LongKey("1.0"),
)
def long_secret_name(self, node=None):
    """Check that secrets work when named using a very long string.
    """
    if node is None:
        node = self.context.node

    key_len=1024

    secret_message = 'secret_password'
    secret_key = 'a'*key_len

    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={secret_key: secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    with When('I use SELECT with a secret'):
        node.query(f"SELECT secret('{secret_key}')",
            message='DB::Exception: Serialization is not implemented', exitcode=48, query_id=query_id)

@TestOutline(Scenario)
@Requirements(
    RQ_SRS_021_Secrets_Backend_InvalidConfig("1.0")
)
@Examples("secret_name",[
    ("", Name("Blank Key"),),
    ("'🔑'", Name("UTF8 Key"),),
],)
def invalid_secrets_config(self, secret_name, node=None):
    """Check that secrets work when named using no characters.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_password'

    query_id = getuid()

    with When("I create an invalid config"):
        secrets_invalid_config(entries={f'{secret_name}': secret_message})

@TestOutline(Scenario)
@Requirements(
    RQ_SRS_021_Secrets_InvalidKeys("1.0"),
)
@Examples("secret_name exitcode",[
    ("''", 47),
    ("'🔑'", 47),
    ("toDecimal256(1,10)", 43),
    ("concat('x','y')", 47),
],)
def select_with_invalid_keys(self, secret_name, exitcode, node=None):
    """Check that SELECT with an invalid key does not cause crash.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_password'

    with When('I use SELECT with a secret'):
        node.query(f"SELECT secret({secret_name})",
            message="Exception: ", exitcode=exitcode)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_ColumnValueName("1.0")
)
def secret_with_column_value_key(self, node=None):
    """Check that using a secret name stored in a table does not work.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_password'
    secret_key = 'secret_key'

    table_name = f'table_{getuid()}'
    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={secret_key: secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    try:
        with And("I have a table"):
            node.query(f"CREATE TABLE {table_name}(x String) Engine= Memory")

        with When("I insert the secret key into the table"):
            node.query(f"INSERT INTO {table_name} VALUES ('{secret_key}')")

        with And("I try to access the secret using the column value"):
            node.query(f"SELECT secret(x) FROM {table_name}",
                query_id=query_id, exitcode=44, message= "Exception: Illegal type of argument #1 ")
        yield

    finally:
        with Finally("I drop the table", flags=TE):
            node.query(f"DROP TABLE IF EXISTS {table_name}")

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_MySQL("1.0"),
)
def mysql_table(self, node=None):
    """Check that secrets can be used to access mysql node.
    """
    table_name = f'table_{getuid()}'
    user_name = f'user_{getuid()}'[0:31]

    if node is None:
        node = self.context.node
    mysql_node = self.context.mysql_node

    secret_message = 'secret_password'

    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'mysql_password': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    with And("I have a table"):
        table(name=table_name, node=node, mysql_node=mysql_node, user_name=user_name, user_password=secret_message)

    with When("I drop the table if exists"):
        node.query(f"DROP TABLE IF EXISTS {table_name}")

    with And("I create the table"):
        sql = f"""
        CREATE TABLE {table_name}
        (
            id UInt8,
            x UInt8
        )
        ENGINE = MySQL('{mysql_node.name}:3306', 'db', '{table_name}', '{user_name}', secret('mysql_password'))
        """
        node.query(textwrap.dedent(sql), query_id=query_id)

    with Then("I insert into the table"):
        node.query(f"SELECT * FROM {table_name}")

    with Finally("I drop the table", flags=TE):
        node.query(f'DROP TABLE IF EXISTS {table_name}')

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_MySQL("1.0"),
)
def mysql_func(self, node=None):
    """Check that secrets can be used to access mysql table function.
    """
    table_name = f'table_{getuid()}'
    user_name = f'user_{getuid()}'[0:31]

    if node is None:
        node = self.context.node
    mysql_node = self.context.mysql_node

    secret_message = 'secret_password'

    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'mysql_password': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    with And("I have a table"):
        table(name=table_name, node=node, mysql_node=mysql_node, user_name=user_name, user_password=secret_message)

    with And("I have values in MySQL table"):
        mysql_node.command(f"MYSQL_PWD=password mysql -D db -u user -e \"INSERT INTO {table_name}(x) VALUES (1)\"", exitcode=0)

    with When("I read from the MySQL table using the table function"):
        node.query(f"SELECT * FROM mysql('{mysql_node.name}:3306', 'db', '{table_name}', '{user_name}', secret('mysql_password'))", query_id=query_id)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_EncryptDecrypt("1.0"),
)
def encrypt_decrypt_secret_key(self, node=None):
    """Check that when secret function is used as the key for the encrypt or decrypt function,
    the content of the secret is not shown in the server log.
    """
    if node is None:
        node = self.context.node

    secret_message = 'passwordpassword'

    query_id_0 = getuid()
    query_id_1 = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'literal_password': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id_0, query_id_1])

    with When('I use encrypt with a secret key'):
        node.query("SELECT encrypt('aes-128-ecb', 'to be encrypted', secret('literal_password'));", query_id=query_id_0)

    with And('I use decrypt with a secret key'):
        node.query("SELECT decrypt('aes-128-ecb', encrypt('aes-128-ecb',  'to be encrypted', secret('literal_password')), secret('literal_password'));", query_id=query_id_1)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_AESEncryptDecrypt("1.0"),
)
def aes_encrypt_decrypt(self, node=None):
    """Check that when secret function is used as the key for the aes_encrypt_mysql or aes_decrypt_mysql function,
    the content of the secret is not shown in the server log.
    """
    if node is None:
        node = self.context.node

    secret_message = 'passwordpassword'

    query_id_0 = getuid()
    query_id_1 = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'literal_password': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id_0, query_id_1])

    with When('I use aes_encrypt_mysql with a secret key'):
        node.query("SELECT aes_encrypt_mysql('aes-128-ecb', 'to be encrypted', secret('literal_password'));", query_id=query_id_0)

    with And('I use aes_decrypt_mysql with a secret key'):
        node.query("SELECT aes_decrypt_mysql('aes-128-ecb', aes_encrypt_mysql('aes-128-ecb',  'to be encrypted', secret('literal_password')), secret('literal_password'));", query_id=query_id_1)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_DataType("1.0"),
)
def encrypt_decrypt_secret_mode(self, node=None):
    """Check that the encrypt function fails because of the Secret data type.
    """
    if node is None:
        node = self.context.node

    secret_message = 'aes-128-ecb'

    query_id_0 = getuid()
    query_id_1 = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'secret_mode': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id_0, query_id_1])

    with When('I use encrypt with a secret mode'):
        node.query("SELECT encrypt(secret('secret_mode'), 'to be encrypted', 'key');",
            exitcode=43, message='DB::Exception: Illegal type of argument #1 \'mode\' of function encrypt, expected encryption mode string, got Secret', query_id=query_id_0)

    with And('I use decrypt with a secret mode'):
        node.query("SELECT decrypt(secret('secret_mode'), encrypt(secret('secret_mode'),  'to be encrypted', 'key'), 'key')",
            exitcode=43, message='DB::Exception: Illegal type of argument #1 \'mode\' of function encrypt, expected encryption mode string, got Secret', query_id=query_id_1)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_Usability("1.0"),
)
def encrypt_decrypt_secret_message(self, node=None):
    """Check that the secret function can be used in a field that doesn't accept secrets and it won't cause a leak.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_words_to_be_encrypted'

    query_id_0 = getuid()
    query_id_1 = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'secret_message': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id_0, query_id_1])

    with When("I use encrypt with a secret mode"):
        node.query("SELECT encrypt('aes-128-ecb', secret('secret_message'), 'passwordpassword');",
            exitcode=43, message='DB::Exception: Illegal type of argument #2 \'input\' of function encrypt, expected plaintext, got Secret', query_id=query_id_0)

    with And('I use decrypt with a secret mode'):
        node.query("SELECT decrypt('aes-128-ecb', encrypt('aes-128-ecb',  secret('secret_message'), 'passwordpassword'), 'passwordpassword')",
            exitcode=43, message='DB::Exception: Illegal type of argument #2 \'input\' of function encrypt, expected plaintext, got Secret', query_id=query_id_1)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_Backend_Plaintext_Subsection("1.0")
)
def multiple_subsections(self, node=None):
    """Check that plaintext secrets can have multiple subsections.
    """
    if node is None:
        node = self.context.node

    query_id_0 = getuid()
    query_id_1 = getuid()

    with Given("I have a config with multiple subsections"):
        secrets_config(entries={'passwords':{'a':'sub_password_a','b':'sub_password_b'}})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message='sub_password', query_ids=[query_id_0, query_id_1])

    with When('I use SELECT with the first secret'):
        node.query("SELECT secret('passwords.a')",
            message='DB::Exception: Serialization is not implemented', exitcode=48, query_id=query_id_0)

    with And('I use SELECT with the second secret'):
        node.query("SELECT secret('passwords.b')",
            message='DB::Exception: Serialization is not implemented', exitcode=48, query_id=query_id_1)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_Backend_Plaintext_Subsection_Wrapping("1.0")
)
def inaccessible_secret_password(self, node=None):
    """Check that secret that is not wrapped properly in the config file is inaccessible.
    """
    if node is None:
        node = self.context.node

    query_id = getuid()

    with Given("I have a config with multiple subsections"):
        secrets_config(entries={'passwords':{'a':'sub_password_a','b':'sub_password_b'}}, inaccessible_secret=True)

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message='inaccessible_password', query_ids=[query_id])

    with When('I use SELECT with the inaccessible secret'):
        node.query("SELECT secret('passwords')",
            message="DB::Exception: Can't get secret value by path", exitcode=47, query_id=query_id)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_Duplicate("1.0")
)
def duplicate_secret_names(self, node=None):
    """Check that ClickHouse runs while having two secrets with the same name.
    """
    if node is None:
        node = self.context.node

    query_id = getuid()

    with Given("I have a config with multiple subsections"):
        secrets_config(entries={'a':'passwordpassword','a':'passwordpassword_b'})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message='passwordpassword', query_ids=[query_id])

    with When('I use SELECT wih the duplicate secret'):
        node.query("SELECT aes_encrypt_mysql('aes-128-ecb', 'to be encrypted', secret('a'))", query_id=query_id)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_CreateUser("1.0")
)
def user_with_secret_password(self, node=None):
    """Check that it is not possible to create a user with a secret password and it doesn't leak.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_user_password'
    user_name = f'user_{getuid()}'

    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'secret_message': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    with When("I create a user with a secret password"):
        node.query(f"CREATE USER {user_name} IDENTIFIED BY secret('secret_message')", query_id=query_id,
            exitcode=62, message="Exception: Syntax error: failed at position 69 ('secret')")

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_Backend_SecretBackend("1.0")
)
def secret_backend(self, node=None):
    """Check that the 'secret backend' keyword works.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_password'

    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'secret_backend': 'plaintext' , 'secret_message': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    with When('I use SELECT with a secret'):
        node.query("SELECT secret('secret_message')",
            message='DB::Exception: Serialization is not implemented', exitcode=48, query_id=query_id)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_Backend_MultipleOptions("1.0")
)
def multiple_options(self, node=None):
    """Check that secrets work with multiple options.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_password'

    query_id_0 = getuid()
    query_id_1 = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'secret_message_0': secret_message , 'secret_message_1': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id_0, query_id_1])

    with When('I use SELECT with the first secret'):
        node.query("SELECT secret('secret_message_0')",
            message='DB::Exception: Serialization is not implemented', exitcode=48, query_id=query_id_0)

    with And('I use SELECT with the second secret'):
        node.query("SELECT secret('secret_message_1')",
            message='DB::Exception: Serialization is not implemented', exitcode=48, query_id=query_id_1)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_ConfigPermission("1.0")
)
def config_permissions(self, node=None):
    """Check config permissions.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_password'

    query_id = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'secret_message_0': secret_message , 'secret_message_1': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id])

    with And("The config file has restricted permissions"):
        node.command("chmod 600 /var/lib/clickhouse/preprocessed_configs/_config.xml")

    with When('I use SELECT with the secret when the config file should not be readable'):
        node.query("SELECT secret('secret_message')",
            message='DB::Exception: Can\'t get secret value by path', exitcode=47, query_id=query_id)

@TestScenario
@Requirements(
    RQ_SRS_021_Secrets_ClientSideExceptions("1.0"),
)
def clientside_exception(self, node=None):
    """Check that client side syntax exceptions don't leak secrets.
    """
    if node is None:
        node = self.context.node

    secret_message = 'secret_password'
    user_name = f'user_{getuid()}'
    table_name = f'table_{getuid()}'

    query_id_0 = getuid()
    query_id_1 = getuid()
    query_id_2 = getuid()

    with Given("I have a secrets config"):
        secrets_config(entries={'secret_key': secret_message})

    with And("I prepare to check the server log"):
        secret_leak_check(secret_message=secret_message, query_ids=[query_id_0, query_id_1, query_id_2])

    with When("I use SELECT with a syntax error"):
        node.query("SELECTs secret('secret_key')",
            message='Exception: Syntax error', exitcode=62, query_id=query_id_0)

    with And("I create a user with a syntax error"):
        node.query(f"CREATE USERs {user_name} IDENTIFIED BY secret('secret_key')",
            message='Exception: Syntax error', exitcode=62, query_id=query_id_1)

    try:
        with Given("I have a table"):
            node.query(f"CREATE TABLE {table_name}(x String) Engine= Memory")

        with When("I insert the secret key into the table with a syntax error"):
            node.query(f"INSERTs INTO {table_name} VALUES (secret('secret_key'))",
                message='Exception: Syntax error', exitcode=62, query_id=query_id_2)
        yield

    finally:
        with Finally("I drop the table", flags=TE):
            node.query(f"DROP TABLE IF EXISTS {table_name}")

@TestSuite
@Requirements(
    RQ_SRS_021_Secrets("1.0")
)
def valid_secrets(self):
    """Check valid use cases for secrets and make sure they don't leak.
    """
    Scenario(run=encrypt_decrypt_secret_key)
    Scenario(run=aes_encrypt_decrypt)
    Scenario(run=mysql_table)
    Scenario(run=mysql_func)

@TestSuite
def invalid_secrets(self):
    """Check invalid use cases for secrets and make sure they don't leak.
    """
    Scenario(run=encrypt_decrypt_secret_mode)
    Scenario(run=encrypt_decrypt_secret_message)
    Scenario(run=user_with_secret_password)
    Scenario(run=clientside_exception)
    Scenario(run=select_with_invalid_keys)
    Scenario(run=secret_with_column_value_key)
    Scenario(run=multiple_inputs)

@TestSuite
@Requirements(
    RQ_SRS_021_Secrets_Backend("1.0")
)
def config(self):
    """Check different config setups.
    """
    Scenario(run=select)
    Scenario(run=utf8_secret_message)
    Scenario(run=long_secret_name)
    Scenario(run=invalid_secrets_config)
    Scenario(run=inaccessible_secret_password)
    Scenario(run=multiple_subsections)
    Scenario(run=duplicate_secret_names)
    Scenario(run=secret_backend)
    Scenario(run=multiple_options)
    Scenario(run=config_permissions)

@TestFeature
@Requirements(
    RQ_SRS_021_Secrets_Hidden("1.0"),
    RQ_SRS_021_Secrets_Backend_Plaintext("1.0"),
)
@Name("feature")
def feature(self, node='clickhouse1', mysql_node="mysql1"):
    """Test encrypt and decrypt functions with secrets.
    """
    self.context.node = self.context.cluster.node(node)
    self.context.mysql_node = self.context.cluster.node(mysql_node)

    for suite in loads(current_module(), Suite):
        Suite(run = suite)
