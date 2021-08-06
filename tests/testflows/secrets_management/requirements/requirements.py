# These requirements were auto generated
# from software requirements specification (SRS)
# document by TestFlows v1.6.210601.1142234.
# Do not edit by hand but re-generate instead
# using 'tfs requirements generate' command.
from testflows.core import Specification
from testflows.core import Requirement

Heading = Specification.Heading

RQ_SRS_021_Secrets = Requirement(
    name='RQ.SRS-021.Secrets',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support a `secret` function to provide sensitive data to\n'
        '[ClickHouse] and to manage sensitive data within [ClickHouse]. The `secret`\n'
        'function SHALL be used directly in the [ClickHouse] SQL dialect similar to\n'
        'the following examples:\n'
        '\n'
        "* `SELECT encrypt('aes-256-cfb128',  'plaintext to encrypt', secret('mysecret'));`\n"
        "* `SELECT * FROM mysql('localhost:3306', 'test', 'test', secret('mysql_localhost_user'), secret('mysql_localhost_password'));`\n"
        "* `SELECT * FROM s3('https://s3.my-region.amazonaws.com/my-bucket', secret('aws_access_key'), secret('aws_secret_access_key'), 'CSVWithNames', 'd UInt64')`\n"
        '\n'
        ),
    link=None,
    level=3,
    num='4.1.1')

RQ_SRS_021_Secrets_Hidden = Requirement(
    name='RQ.SRS-021.Secrets.Hidden',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL hide all secrets in any saved data. Secrets SHALL not be\n'
        'shown to any database user, and SHALL not be visible in any log files.\n'
        '\n'
        ),
    link=None,
    level=3,
    num='4.2.1')

RQ_SRS_021_Secrets_Usability = Requirement(
    name='RQ.SRS-021.Secrets.Usability',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL be able to use secrets in any place in a query.\n'
        '\n'
        ),
    link=None,
    level=3,
    num='4.3.1')

RQ_SRS_021_Secrets_DataType = Requirement(
    name='RQ.SRS-021.Secrets.DataType',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL return a special data type `Secret(String)` when using the secrets function.\n'
        '\n'
        ),
    link=None,
    level=3,
    num='4.4.1')

RQ_SRS_021_Secrets_Select = Requirement(
    name='RQ.SRS-021.Secrets.Select',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL not make secrets accessible through the `SELECT` function.\n'
        '\n'
        ),
    link=None,
    level=3,
    num='4.5.1')

RQ_SRS_021_Secrets_Syntax = Requirement(
    name='RQ.SRS-021.Secrets.Syntax',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `secret` function with the following syntax:\n'
        '\n'
        "`secret('mysecret')`\n"
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.1.1')

RQ_SRS_021_Secrets_EncryptDecrypt = Requirement(
    name='RQ.SRS-021.Secrets.EncryptDecrypt',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `secret` functions to be used as the key for encrypt and decrypt functions.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.2.1')

RQ_SRS_021_Secrets_AESEncryptDecrypt = Requirement(
    name='RQ.SRS-021.Secrets.AESEncryptDecrypt',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `secret` functions to be used as the key for aes encrypt and aes decrypt functions.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.3.1')

RQ_SRS_021_Secrets_MySQL = Requirement(
    name='RQ.SRS-021.Secrets.MySQL',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support using secrets to connect to MySQL service.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.4.1')

RQ_SRS_021_Secrets_Duplicate = Requirement(
    name='RQ.SRS-021.Secrets.Duplicate',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support having multiple secrets with the same name but will only use the first one.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.5.1')

RQ_SRS_021_Secrets_UTF8Message = Requirement(
    name='RQ.SRS-021.Secrets.UTF8Message',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support using utf-8 characters as the secret message.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.6.1')

RQ_SRS_021_Secrets_LongKey = Requirement(
    name='RQ.SRS-021.Secrets.LongKey',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support using a key with 1024 characters.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.7.1')

RQ_SRS_021_Secrets_InvalidKeys = Requirement(
    name='RQ.SRS-021.Secrets.InvalidKeys',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL not crash or leak secrets when an invalid secrets name is used.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.8.1')

RQ_SRS_021_Secrets_ClientSideExceptions = Requirement(
    name='RQ.SRS-021.Secrets.ClientSideExceptions',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL not leak secrets content through client side exceptions.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.9.1')

RQ_SRS_021_Secrets_ColumnValueName = Requirement(
    name='RQ.SRS-021.Secrets.ColumnValueName',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL not support using the `secrets` function using values from a table.\n'
        'For example,\n'
        '```sql\n'
        'SELECT secret(x) FROM table\n'
        '```\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.10.1')

RQ_SRS_021_Secrets_CreateUser = Requirement(
    name='RQ.SRS-021.Secrets.CreateUser',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL not support creating a user indentified by a Secret password.\n'
        'Example,\n'
        '```sql\n'
        "CREATE USER user_name IDENTIFIED BY secret('secret_message')\n"
        '```\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.6.11.1')

RQ_SRS_021_Secrets_ConfigPermission = Requirement(
    name='RQ.SRS-021.Secrets.ConfigPermission',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL not be able to access `secrets.xml` if it does not have permissions to the file.\n'
        '\n'
        ),
    link=None,
    level=3,
    num='4.7.1')

RQ_SRS_021_Secrets_Backend = Requirement(
    name='RQ.SRS-021.Secrets.Backend',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support up to 3 different methods of backend support for the\n'
        '`secret` function. Secrets used by the `secret` function can be accessed via\n'
        'a plaintext name, via a key-derivation function, or stored and accessed using a\n'
        '[HashiCorp vault]. These backend support options SHALL be configured in the\n'
        '`<secrets>` section of the config.xml file, or a separate file in the config.d\n'
        'directory.\n'
        '\n'
        ),
    link=None,
    level=3,
    num='4.8.1')

RQ_SRS_021_Secrets_Backend_InvalidConfig = Requirement(
    name='RQ.SRS-021.Secrets.Backend.InvalidConfig',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL NOT support using certain characters for secret keys.\n'
        'For example,\n'
        '* Blank Key\n'
        '* UTF8 Key\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.2.1')

RQ_SRS_021_Secrets_Backend_Plaintext = Requirement(
    name='RQ.SRS-021.Secrets.Backend.Plaintext',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support plaintext storage of secrets used by the `secret`\n'
        'function in the `<secrets>` section of the config.xml file, or a separate file\n'
        'in the config.d directory with syntax similar to the following:\n'
        '\n'
        '```xml\n'
        '<yandex>\n'
        '    <secrets>\n'
        '        <!-- plaintext secret, accessible by name `literal_password` -->\n'
        '        <literal_password>this is the password</literal_password>\n'
        '    </secrets>\n'
        '</yandex>\n'
        '```\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.3.1')

RQ_SRS_021_Secrets_Backend_Plaintext_Subsection = Requirement(
    name='RQ.SRS-021.Secrets.Backend.Plaintext.Subsection',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support adding subsections to a secret stored as plaintext\n'
        'similar to the following:\n'
        '\n'
        '```xml\n'
        '<yandex>\n'
        '    <secrets>\n'
        '        <!-- plaintext secret, accessible by name `passwords` -->\n'
        '        <passwords>\n'
        '            <a>password_a</a>\n'
        '            <b>password_b</b>\n'
        '            <c><d>password_c_d</d></c>\n'
        '        </passwords>\n'
        '    </secrets>\n'
        '</yandex>\n'
        '```\n'
        '\n'
        'These passwords SHALL be accessed as `passwords.a`, `passwords.b`, and\n'
        '`passwords.c.d` respectively.\n'
        '\n'
        "Ex: `SELECT secret('passwords.a');`\n"
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.3.2')

RQ_SRS_021_Secrets_Backend_Plaintext_Subsection_Wrapping = Requirement(
    name='RQ.SRS-021.Secrets.Backend.Plaintext.Subsection.Wrapping',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL require any secret with subsections to wrap each subsection\n'
        'in tags. Any subsection not wrapped in tags SHALL be inaccessible. See the\n'
        'following example:\n'
        '\n'
        '```xml\n'
        '<yandex>\n'
        '    <secrets>\n'
        '        <!-- plaintext secret, accessible by name `passwords` -->\n'
        '        <passwords>\n'
        '            <a>password_a</a>\n'
        '            <b>password_b</b>\n'
        '            <c><d>password_c_d</d></c>\n'
        '            inaccessible_password\n'
        '        </passwords>\n'
        '    </secrets>\n'
        '</yandex>\n'
        '```\n'
        '\n'
        "In this example, the data 'inaccessible_password' cannot be retrieved, and\n"
        '[ClickHouse] SHALL throw an exception upon entry of the following command:\n'
        '\n'
        "`SELECT secret('passwords');`\n"
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.3.3')

RQ_SRS_021_Secrets_Backend_KeyDerivation = Requirement(
    name='RQ.SRS-021.Secrets.Backend.KeyDerivation',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support storage and access of secrets used by the `secret`\n'
        'function by key-derivation hashing configured in the `<secrets>` section of the\n'
        'config.xml file, or a separate file in the config.d directory with syntax similar\n'
        'to the following:\n'
        '\n'
        '```xml\n'
        '<yandex>\n'
        '    <secrets>\n'
        '        <!-- server-key-derivation-based secret, accessible by name `server_key_32` -->\n'
        '        <server_key_32>\n'
        '            <secret_backend>server_key_provider</secret_backend>\n'
        '            <server_key_provider>\n'
        '                <key_bytes>32</key_bytes>\n'
        '            </server_key_provider>\n'
        '        </server_key_32>\n'
        '    </secrets>\n'
        '</yandex>\n'
        '```\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.4.1')

RQ_SRS_021_Secrets_Backend_KeyDerivation_KeyBytes = Requirement(
    name='RQ.SRS-021.Secrets.Backend.KeyDerivation.KeyBytes',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `<key_bytes>` subsection of the\n'
        '`<server_key_provider>` section of secrets defined in the `<secrets>`\n'
        'section using the server-key-derivation backend to specify the number of bytes\n'
        'in the access key.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.4.2')

RQ_SRS_021_Secrets_Backend_HashiCorp = Requirement(
    name='RQ.SRS-021.Secrets.Backend.HashiCorp',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support storage and access of secrets used by the `secret`\n'
        'function in a [HashiCorp vault] configured in the `<secrets>` section of the\n'
        'config.xml file, or a separate file in the config.d directory with syntax\n'
        'similar to the following:\n'
        '\n'
        '```xml\n'
        '<secrets>\n'
        '    <!-- hashicorp-vault-managed secret, accessible by name `external_secret` -->\n'
        '    <external_secret>\n'
        '        <secret_backend>hashicorp_vault</secret_backend>\n'
        '        <hashicorp_vault>\n'
        '            <token>my-token</token>\n'
        '            <namespace>my-namespace</namespace>\n'
        '            <vault_host>my-vault-url</vault_host>\n'
        '            <secret_name>my-secret-name</secret_name>\n'
        '        </hashicorp_vault>\n'
        '    </external_secret>\n'
        '</secrets>\n'
        '```\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.5.1')

RQ_SRS_021_Secrets_Backend_HashiCorp_Token = Requirement(
    name='RQ.SRS-021.Secrets.Backend.HashiCorp.Token',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `<token>` subsection of the `<hashicorp_vault>`\n'
        'section of secrets defined in the `<secrets>` section using the\n'
        '[HashiCorp vault] backend to specify the vault token.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.5.2')

RQ_SRS_021_Secrets_Backend_HashiCorp_Namespace = Requirement(
    name='RQ.SRS-021.Secrets.Backend.HashiCorp.Namespace',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `<namespace>` subsection of the `<hashicorp_vault>`\n'
        'section of secrets defined in the `<secrets>` section using the [HashiCorp vault]\n'
        'backend to specify the vault namespace.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.5.3')

RQ_SRS_021_Secrets_Backend_HashiCorp_VaultHost = Requirement(
    name='RQ.SRS-021.Secrets.Backend.HashiCorp.VaultHost',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `<vault_host>` subsection of the `<hashicorp_vault>`\n'
        'section of secrets defined in the `<secrets>` section using the [HashiCorp vault]\n'
        'backend to specify the vault host.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.5.4')

RQ_SRS_021_Secrets_Backend_HashiCorp_SecretName = Requirement(
    name='RQ.SRS-021.Secrets.Backend.HashiCorp.SecretName',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `<secret_name>` subsection of the `<hashicorp_vault>`\n'
        'section of secrets defined in the `<secrets>` section using the [HashiCorp vault]\n'
        'backend to specify the name of the secret within the vault.\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.5.5')

RQ_SRS_021_Secrets_Backend_MultipleOptions = Requirement(
    name='RQ.SRS-021.Secrets.Backend.MultipleOptions',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support simultaneous configuration of one or more options\n'
        'for backend support of the `secrets` function at the same time using syntax\n'
        'similar to the following:\n'
        '\n'
        '```xml\n'
        '<yandex>\n'
        '    <secrets>\n'
        '        <!-- plaintext secret, accessible by name `literal_password` -->\n'
        '        <literal_password>this is the password</literal_password>\n'
        '\n'
        '        <!-- server-key-derivation-based secret, accessible by name `server_key_32` -->\n'
        '        <server_key_32>\n'
        '            <secret_backend>server_key_provider</secret_backend>\n'
        '            <server_key_provider>\n'
        '                <key_bytes>32</key_bytes>\n'
        '            </server_key_provider>\n'
        '        </server_key_32>\n'
        '\n'
        '        <!-- hashicorp-vault-managed secret, accessible by name `external_secret` -->\n'
        '        <external_secret>\n'
        '            <secret_backend>hashicorp_vault</secret_backend>\n'
        '            <hashicorp_vault>\n'
        '                <token>my-token</token>\n'
        '                <namespace>my-namespace</namespace>\n'
        '                <vault_host>my-vault-url</vault_host>\n'
        '                <secret_name>my-secret-name</secret_name>\n'
        '            </hashicorp_vault>\n'
        '        </external_secret>\n'
        '    </secrets>\n'
        '</yandex>\n'
        '```\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.6.1')

RQ_SRS_021_Secrets_Backend_SecretBackend = Requirement(
    name='RQ.SRS-021.Secrets.Backend.SecretBackend',
    version='1.0',
    priority=None,
    group=None,
    type=None,
    uid=None,
    description=(
        '[ClickHouse] SHALL support the `<secret_backend>` section as a subsection of\n'
        'any secret to indicate which backend is provided for that secret.\n'
        '\n'
        '```xml\n'
        '<yandex>\n'
        '    <secrets>\n'
        '        <!-- The secret_backend tag indicates that this is a hashicorp vault -->\n'
        '        <external_secret>\n'
        '            <secret_backend>hashicorp_vault</secret_backend>\n'
        '            <hashicorp_vault>\n'
        '                <token>my-token</token>\n'
        '                <namespace>my-namespace</namespace>\n'
        '                <vault_host>my-vault-url</vault_host>\n'
        '                <secret_name>my-secret-name</secret_name>\n'
        '            </hashicorp_vault>\n'
        '        </external_secret>\n'
        '    </secrets>\n'
        '</yandex>\n'
        '```\n'
        '\n'
        ),
    link=None,
    level=4,
    num='4.8.7.1')

SRS_021_ClickHouse_Secrets_Management = Specification(
    name='SRS-021 ClickHouse Secrets Management', 
    description=None,
    author='mtkachenko',
    date='April 12, 2021', 
    status='-', 
    approved_by='-',
    approved_date='-',
    approved_version='-',
    version=None,
    group=None,
    type=None,
    link=None,
    uid=None,
    parent=None,
    children=None,
    headings=(
        Heading(name='Revision History', level=1, num='1'),
        Heading(name='Introduction', level=1, num='2'),
        Heading(name='Terminology', level=1, num='3'),
        Heading(name='Requirements', level=1, num='4'),
        Heading(name='`SECRET` Function', level=2, num='4.1'),
        Heading(name='RQ.SRS-021.Secrets', level=3, num='4.1.1'),
        Heading(name='Hidden', level=2, num='4.2'),
        Heading(name='RQ.SRS-021.Secrets.Hidden', level=3, num='4.2.1'),
        Heading(name='Usability', level=2, num='4.3'),
        Heading(name='RQ.SRS-021.Secrets.Usability', level=3, num='4.3.1'),
        Heading(name='Data Type', level=2, num='4.4'),
        Heading(name='RQ.SRS-021.Secrets.DataType', level=3, num='4.4.1'),
        Heading(name='Select', level=2, num='4.5'),
        Heading(name='RQ.SRS-021.Secrets.Select', level=3, num='4.5.1'),
        Heading(name='Specific', level=2, num='4.6'),
        Heading(name='Syntax', level=3, num='4.6.1'),
        Heading(name='RQ.SRS-021.Secrets.Syntax', level=4, num='4.6.1.1'),
        Heading(name='Encrypt/Decrypt', level=3, num='4.6.2'),
        Heading(name='RQ.SRS-021.Secrets.EncryptDecrypt', level=4, num='4.6.2.1'),
        Heading(name='AES Encrypt/Decrypt', level=3, num='4.6.3'),
        Heading(name='RQ.SRS-021.Secrets.AESEncryptDecrypt', level=4, num='4.6.3.1'),
        Heading(name='MySQL', level=3, num='4.6.4'),
        Heading(name='RQ.SRS-021.Secrets.MySQL', level=4, num='4.6.4.1'),
        Heading(name='Duplicate', level=3, num='4.6.5'),
        Heading(name='RQ.SRS-021.Secrets.Duplicate', level=4, num='4.6.5.1'),
        Heading(name='UTF-8 Message', level=3, num='4.6.6'),
        Heading(name='RQ.SRS-021.Secrets.UTF8Message', level=4, num='4.6.6.1'),
        Heading(name='Long Key', level=3, num='4.6.7'),
        Heading(name='RQ.SRS-021.Secrets.LongKey', level=4, num='4.6.7.1'),
        Heading(name='Invalid Keys', level=3, num='4.6.8'),
        Heading(name='RQ.SRS-021.Secrets.InvalidKeys', level=4, num='4.6.8.1'),
        Heading(name='Client Side Exceptions', level=3, num='4.6.9'),
        Heading(name='RQ.SRS-021.Secrets.ClientSideExceptions', level=4, num='4.6.9.1'),
        Heading(name='Column Value Name', level=3, num='4.6.10'),
        Heading(name='RQ.SRS-021.Secrets.ColumnValueName', level=4, num='4.6.10.1'),
        Heading(name='Create User', level=3, num='4.6.11'),
        Heading(name='RQ.SRS-021.Secrets.CreateUser', level=4, num='4.6.11.1'),
        Heading(name='Config Permission', level=2, num='4.7'),
        Heading(name='RQ.SRS-021.Secrets.ConfigPermission', level=3, num='4.7.1'),
        Heading(name='Backend', level=2, num='4.8'),
        Heading(name='RQ.SRS-021.Secrets.Backend', level=3, num='4.8.1'),
        Heading(name='Invalid Config', level=3, num='4.8.2'),
        Heading(name='RQ.SRS-021.Secrets.Backend.InvalidConfig', level=4, num='4.8.2.1'),
        Heading(name='Plaintext', level=3, num='4.8.3'),
        Heading(name='RQ.SRS-021.Secrets.Backend.Plaintext', level=4, num='4.8.3.1'),
        Heading(name='RQ.SRS-021.Secrets.Backend.Plaintext.Subsection', level=4, num='4.8.3.2'),
        Heading(name='RQ.SRS-021.Secrets.Backend.Plaintext.Subsection.Wrapping', level=4, num='4.8.3.3'),
        Heading(name='Key Derivation', level=3, num='4.8.4'),
        Heading(name='RQ.SRS-021.Secrets.Backend.KeyDerivation', level=4, num='4.8.4.1'),
        Heading(name='RQ.SRS-021.Secrets.Backend.KeyDerivation.KeyBytes', level=4, num='4.8.4.2'),
        Heading(name='HashiCorp', level=3, num='4.8.5'),
        Heading(name='RQ.SRS-021.Secrets.Backend.HashiCorp', level=4, num='4.8.5.1'),
        Heading(name='RQ.SRS-021.Secrets.Backend.HashiCorp.Token', level=4, num='4.8.5.2'),
        Heading(name='RQ.SRS-021.Secrets.Backend.HashiCorp.Namespace', level=4, num='4.8.5.3'),
        Heading(name='RQ.SRS-021.Secrets.Backend.HashiCorp.VaultHost', level=4, num='4.8.5.4'),
        Heading(name='RQ.SRS-021.Secrets.Backend.HashiCorp.SecretName', level=4, num='4.8.5.5'),
        Heading(name='MultipleOptions', level=3, num='4.8.6'),
        Heading(name='RQ.SRS-021.Secrets.Backend.MultipleOptions', level=4, num='4.8.6.1'),
        Heading(name='Secret Backend', level=3, num='4.8.7'),
        Heading(name='RQ.SRS-021.Secrets.Backend.SecretBackend', level=4, num='4.8.7.1'),
        Heading(name='References', level=1, num='5'),
        ),
    requirements=(
        RQ_SRS_021_Secrets,
        RQ_SRS_021_Secrets_Hidden,
        RQ_SRS_021_Secrets_Usability,
        RQ_SRS_021_Secrets_DataType,
        RQ_SRS_021_Secrets_Select,
        RQ_SRS_021_Secrets_Syntax,
        RQ_SRS_021_Secrets_EncryptDecrypt,
        RQ_SRS_021_Secrets_AESEncryptDecrypt,
        RQ_SRS_021_Secrets_MySQL,
        RQ_SRS_021_Secrets_Duplicate,
        RQ_SRS_021_Secrets_UTF8Message,
        RQ_SRS_021_Secrets_LongKey,
        RQ_SRS_021_Secrets_InvalidKeys,
        RQ_SRS_021_Secrets_ClientSideExceptions,
        RQ_SRS_021_Secrets_ColumnValueName,
        RQ_SRS_021_Secrets_CreateUser,
        RQ_SRS_021_Secrets_ConfigPermission,
        RQ_SRS_021_Secrets_Backend,
        RQ_SRS_021_Secrets_Backend_InvalidConfig,
        RQ_SRS_021_Secrets_Backend_Plaintext,
        RQ_SRS_021_Secrets_Backend_Plaintext_Subsection,
        RQ_SRS_021_Secrets_Backend_Plaintext_Subsection_Wrapping,
        RQ_SRS_021_Secrets_Backend_KeyDerivation,
        RQ_SRS_021_Secrets_Backend_KeyDerivation_KeyBytes,
        RQ_SRS_021_Secrets_Backend_HashiCorp,
        RQ_SRS_021_Secrets_Backend_HashiCorp_Token,
        RQ_SRS_021_Secrets_Backend_HashiCorp_Namespace,
        RQ_SRS_021_Secrets_Backend_HashiCorp_VaultHost,
        RQ_SRS_021_Secrets_Backend_HashiCorp_SecretName,
        RQ_SRS_021_Secrets_Backend_MultipleOptions,
        RQ_SRS_021_Secrets_Backend_SecretBackend,
        ),
    content='''
# SRS-021 ClickHouse Secrets Management
# Software Requirements Specification

(c) 2021 Altinity LTD. All Rights Reserved.

**Document status:** Confidential

**Author:** mtkachenko

**Date:** April 12, 2021

## Approval

**Status:** -

**Version:** -

**Approved by:** -

**Date:** -

## Table of Contents

* 1 [Revision History](#revision-history)
* 2 [Introduction](#introduction)
* 3 [Terminology](#terminology)
* 4 [Requirements](#requirements)
  * 4.1 [`SECRET` Function](#secret-function)
    * 4.1.1 [RQ.SRS-021.Secrets](#rqsrs-021secrets)
  * 4.2 [Hidden](#hidden)
    * 4.2.1 [RQ.SRS-021.Secrets.Hidden](#rqsrs-021secretshidden)
  * 4.3 [Usability](#usability)
    * 4.3.1 [RQ.SRS-021.Secrets.Usability](#rqsrs-021secretsusability)
  * 4.4 [Data Type](#data-type)
    * 4.4.1 [RQ.SRS-021.Secrets.DataType](#rqsrs-021secretsdatatype)
  * 4.5 [Select](#select)
    * 4.5.1 [RQ.SRS-021.Secrets.Select](#rqsrs-021secretsselect)
  * 4.6 [Specific](#specific)
    * 4.6.1 [Syntax](#syntax)
      * 4.6.1.1 [RQ.SRS-021.Secrets.Syntax](#rqsrs-021secretssyntax)
    * 4.6.2 [Encrypt/Decrypt](#encryptdecrypt)
      * 4.6.2.1 [RQ.SRS-021.Secrets.EncryptDecrypt](#rqsrs-021secretsencryptdecrypt)
    * 4.6.3 [AES Encrypt/Decrypt](#aes-encryptdecrypt)
      * 4.6.3.1 [RQ.SRS-021.Secrets.AESEncryptDecrypt](#rqsrs-021secretsaesencryptdecrypt)
    * 4.6.4 [MySQL](#mysql)
      * 4.6.4.1 [RQ.SRS-021.Secrets.MySQL](#rqsrs-021secretsmysql)
    * 4.6.5 [Duplicate](#duplicate)
      * 4.6.5.1 [RQ.SRS-021.Secrets.Duplicate](#rqsrs-021secretsduplicate)
    * 4.6.6 [UTF-8 Message](#utf-8-message)
      * 4.6.6.1 [RQ.SRS-021.Secrets.UTF8Message](#rqsrs-021secretsutf8message)
    * 4.6.7 [Long Key](#long-key)
      * 4.6.7.1 [RQ.SRS-021.Secrets.LongKey](#rqsrs-021secretslongkey)
    * 4.6.8 [Invalid Keys](#invalid-keys)
      * 4.6.8.1 [RQ.SRS-021.Secrets.InvalidKeys](#rqsrs-021secretsinvalidkeys)
    * 4.6.9 [Client Side Exceptions](#client-side-exceptions)
      * 4.6.9.1 [RQ.SRS-021.Secrets.ClientSideExceptions](#rqsrs-021secretsclientsideexceptions)
    * 4.6.10 [Column Value Name](#column-value-name)
      * 4.6.10.1 [RQ.SRS-021.Secrets.ColumnValueName](#rqsrs-021secretscolumnvaluename)
    * 4.6.11 [Create User](#create-user)
      * 4.6.11.1 [RQ.SRS-021.Secrets.CreateUser](#rqsrs-021secretscreateuser)
  * 4.7 [Config Permission](#config-permission)
    * 4.7.1 [RQ.SRS-021.Secrets.ConfigPermission](#rqsrs-021secretsconfigpermission)
  * 4.8 [Backend](#backend)
    * 4.8.1 [RQ.SRS-021.Secrets.Backend](#rqsrs-021secretsbackend)
    * 4.8.2 [Invalid Config](#invalid-config)
      * 4.8.2.1 [RQ.SRS-021.Secrets.Backend.InvalidConfig](#rqsrs-021secretsbackendinvalidconfig)
    * 4.8.3 [Plaintext](#plaintext)
      * 4.8.3.1 [RQ.SRS-021.Secrets.Backend.Plaintext](#rqsrs-021secretsbackendplaintext)
      * 4.8.3.2 [RQ.SRS-021.Secrets.Backend.Plaintext.Subsection](#rqsrs-021secretsbackendplaintextsubsection)
      * 4.8.3.3 [RQ.SRS-021.Secrets.Backend.Plaintext.Subsection.Wrapping](#rqsrs-021secretsbackendplaintextsubsectionwrapping)
    * 4.8.4 [Key Derivation](#key-derivation)
      * 4.8.4.1 [RQ.SRS-021.Secrets.Backend.KeyDerivation](#rqsrs-021secretsbackendkeyderivation)
      * 4.8.4.2 [RQ.SRS-021.Secrets.Backend.KeyDerivation.KeyBytes](#rqsrs-021secretsbackendkeyderivationkeybytes)
    * 4.8.5 [HashiCorp](#hashicorp)
      * 4.8.5.1 [RQ.SRS-021.Secrets.Backend.HashiCorp](#rqsrs-021secretsbackendhashicorp)
      * 4.8.5.2 [RQ.SRS-021.Secrets.Backend.HashiCorp.Token](#rqsrs-021secretsbackendhashicorptoken)
      * 4.8.5.3 [RQ.SRS-021.Secrets.Backend.HashiCorp.Namespace](#rqsrs-021secretsbackendhashicorpnamespace)
      * 4.8.5.4 [RQ.SRS-021.Secrets.Backend.HashiCorp.VaultHost](#rqsrs-021secretsbackendhashicorpvaulthost)
      * 4.8.5.5 [RQ.SRS-021.Secrets.Backend.HashiCorp.SecretName](#rqsrs-021secretsbackendhashicorpsecretname)
    * 4.8.6 [MultipleOptions](#multipleoptions)
      * 4.8.6.1 [RQ.SRS-021.Secrets.Backend.MultipleOptions](#rqsrs-021secretsbackendmultipleoptions)
    * 4.8.7 [Secret Backend](#secret-backend)
      * 4.8.7.1 [RQ.SRS-021.Secrets.Backend.SecretBackend](#rqsrs-021secretsbackendsecretbackend)
* 5 [References](#references)

## Revision History

This document is stored in an electronic form using [Git] source control
management software hosted in a [GitLab Repository]. All the updates are tracked
using the [Revision History].

## Introduction

## Terminology

* **Part** -
  atomic data part from [ClickHouse] prospective, produced either by insert or merge process.
  Stored as a separate directory at the file system.

## Requirements

### `SECRET` Function

#### RQ.SRS-021.Secrets
version: 1.0

[ClickHouse] SHALL support a `secret` function to provide sensitive data to
[ClickHouse] and to manage sensitive data within [ClickHouse]. The `secret`
function SHALL be used directly in the [ClickHouse] SQL dialect similar to
the following examples:

* `SELECT encrypt('aes-256-cfb128',  'plaintext to encrypt', secret('mysecret'));`
* `SELECT * FROM mysql('localhost:3306', 'test', 'test', secret('mysql_localhost_user'), secret('mysql_localhost_password'));`
* `SELECT * FROM s3('https://s3.my-region.amazonaws.com/my-bucket', secret('aws_access_key'), secret('aws_secret_access_key'), 'CSVWithNames', 'd UInt64')`

### Hidden

#### RQ.SRS-021.Secrets.Hidden
version: 1.0

[ClickHouse] SHALL hide all secrets in any saved data. Secrets SHALL not be
shown to any database user, and SHALL not be visible in any log files.

### Usability

#### RQ.SRS-021.Secrets.Usability
version: 1.0

[ClickHouse] SHALL be able to use secrets in any place in a query.

### Data Type

#### RQ.SRS-021.Secrets.DataType
version: 1.0

[ClickHouse] SHALL return a special data type `Secret(String)` when using the secrets function.

### Select

#### RQ.SRS-021.Secrets.Select
version: 1.0

[ClickHouse] SHALL not make secrets accessible through the `SELECT` function.

### Specific

#### Syntax

##### RQ.SRS-021.Secrets.Syntax
version: 1.0

[ClickHouse] SHALL support the `secret` function with the following syntax:

`secret('mysecret')`

#### Encrypt/Decrypt

##### RQ.SRS-021.Secrets.EncryptDecrypt
version: 1.0

[ClickHouse] SHALL support the `secret` functions to be used as the key for encrypt and decrypt functions.

#### AES Encrypt/Decrypt

##### RQ.SRS-021.Secrets.AESEncryptDecrypt
version: 1.0

[ClickHouse] SHALL support the `secret` functions to be used as the key for aes encrypt and aes decrypt functions.

#### MySQL

##### RQ.SRS-021.Secrets.MySQL
version: 1.0

[ClickHouse] SHALL support using secrets to connect to MySQL service.

#### Duplicate

##### RQ.SRS-021.Secrets.Duplicate
version: 1.0

[ClickHouse] SHALL support having multiple secrets with the same name but will only use the first one.

#### UTF-8 Message

##### RQ.SRS-021.Secrets.UTF8Message
version: 1.0

[ClickHouse] SHALL support using utf-8 characters as the secret message.

#### Long Key

##### RQ.SRS-021.Secrets.LongKey
version: 1.0

[ClickHouse] SHALL support using a key with 1024 characters.

#### Invalid Keys

##### RQ.SRS-021.Secrets.InvalidKeys
version: 1.0

[ClickHouse] SHALL not crash or leak secrets when an invalid secrets name is used.

#### Client Side Exceptions

##### RQ.SRS-021.Secrets.ClientSideExceptions
version: 1.0

[ClickHouse] SHALL not leak secrets content through client side exceptions.

#### Column Value Name

##### RQ.SRS-021.Secrets.ColumnValueName
version: 1.0

[ClickHouse] SHALL not support using the `secrets` function using values from a table.
For example,
```sql
SELECT secret(x) FROM table
```

#### Create User

##### RQ.SRS-021.Secrets.CreateUser
version: 1.0

[ClickHouse] SHALL not support creating a user indentified by a Secret password.
Example,
```sql
CREATE USER user_name IDENTIFIED BY secret('secret_message')
```

### Config Permission

#### RQ.SRS-021.Secrets.ConfigPermission
version: 1.0

[ClickHouse] SHALL not be able to access `secrets.xml` if it does not have permissions to the file.

### Backend

#### RQ.SRS-021.Secrets.Backend
version: 1.0

[ClickHouse] SHALL support up to 3 different methods of backend support for the
`secret` function. Secrets used by the `secret` function can be accessed via
a plaintext name, via a key-derivation function, or stored and accessed using a
[HashiCorp vault]. These backend support options SHALL be configured in the
`<secrets>` section of the config.xml file, or a separate file in the config.d
directory.

#### Invalid Config

##### RQ.SRS-021.Secrets.Backend.InvalidConfig
version: 1.0

[ClickHouse] SHALL NOT support using certain characters for secret keys.
For example,
* Blank Key
* UTF8 Key

#### Plaintext

##### RQ.SRS-021.Secrets.Backend.Plaintext
version: 1.0

[ClickHouse] SHALL support plaintext storage of secrets used by the `secret`
function in the `<secrets>` section of the config.xml file, or a separate file
in the config.d directory with syntax similar to the following:

```xml
<yandex>
    <secrets>
        <!-- plaintext secret, accessible by name `literal_password` -->
        <literal_password>this is the password</literal_password>
    </secrets>
</yandex>
```

##### RQ.SRS-021.Secrets.Backend.Plaintext.Subsection
version: 1.0

[ClickHouse] SHALL support adding subsections to a secret stored as plaintext
similar to the following:

```xml
<yandex>
    <secrets>
        <!-- plaintext secret, accessible by name `passwords` -->
        <passwords>
            <a>password_a</a>
            <b>password_b</b>
            <c><d>password_c_d</d></c>
        </passwords>
    </secrets>
</yandex>
```

These passwords SHALL be accessed as `passwords.a`, `passwords.b`, and
`passwords.c.d` respectively.

Ex: `SELECT secret('passwords.a');`

##### RQ.SRS-021.Secrets.Backend.Plaintext.Subsection.Wrapping
version: 1.0

[ClickHouse] SHALL require any secret with subsections to wrap each subsection
in tags. Any subsection not wrapped in tags SHALL be inaccessible. See the
following example:

```xml
<yandex>
    <secrets>
        <!-- plaintext secret, accessible by name `passwords` -->
        <passwords>
            <a>password_a</a>
            <b>password_b</b>
            <c><d>password_c_d</d></c>
            inaccessible_password
        </passwords>
    </secrets>
</yandex>
```

In this example, the data 'inaccessible_password' cannot be retrieved, and
[ClickHouse] SHALL throw an exception upon entry of the following command:

`SELECT secret('passwords');`

#### Key Derivation

##### RQ.SRS-021.Secrets.Backend.KeyDerivation
version: 1.0

[ClickHouse] SHALL support storage and access of secrets used by the `secret`
function by key-derivation hashing configured in the `<secrets>` section of the
config.xml file, or a separate file in the config.d directory with syntax similar
to the following:

```xml
<yandex>
    <secrets>
        <!-- server-key-derivation-based secret, accessible by name `server_key_32` -->
        <server_key_32>
            <secret_backend>server_key_provider</secret_backend>
            <server_key_provider>
                <key_bytes>32</key_bytes>
            </server_key_provider>
        </server_key_32>
    </secrets>
</yandex>
```

##### RQ.SRS-021.Secrets.Backend.KeyDerivation.KeyBytes
version: 1.0

[ClickHouse] SHALL support the `<key_bytes>` subsection of the
`<server_key_provider>` section of secrets defined in the `<secrets>`
section using the server-key-derivation backend to specify the number of bytes
in the access key.

#### HashiCorp

##### RQ.SRS-021.Secrets.Backend.HashiCorp
version: 1.0

[ClickHouse] SHALL support storage and access of secrets used by the `secret`
function in a [HashiCorp vault] configured in the `<secrets>` section of the
config.xml file, or a separate file in the config.d directory with syntax
similar to the following:

```xml
<secrets>
    <!-- hashicorp-vault-managed secret, accessible by name `external_secret` -->
    <external_secret>
        <secret_backend>hashicorp_vault</secret_backend>
        <hashicorp_vault>
            <token>my-token</token>
            <namespace>my-namespace</namespace>
            <vault_host>my-vault-url</vault_host>
            <secret_name>my-secret-name</secret_name>
        </hashicorp_vault>
    </external_secret>
</secrets>
```

##### RQ.SRS-021.Secrets.Backend.HashiCorp.Token
version: 1.0

[ClickHouse] SHALL support the `<token>` subsection of the `<hashicorp_vault>`
section of secrets defined in the `<secrets>` section using the
[HashiCorp vault] backend to specify the vault token.

##### RQ.SRS-021.Secrets.Backend.HashiCorp.Namespace
version: 1.0

[ClickHouse] SHALL support the `<namespace>` subsection of the `<hashicorp_vault>`
section of secrets defined in the `<secrets>` section using the [HashiCorp vault]
backend to specify the vault namespace.

##### RQ.SRS-021.Secrets.Backend.HashiCorp.VaultHost
version: 1.0

[ClickHouse] SHALL support the `<vault_host>` subsection of the `<hashicorp_vault>`
section of secrets defined in the `<secrets>` section using the [HashiCorp vault]
backend to specify the vault host.

##### RQ.SRS-021.Secrets.Backend.HashiCorp.SecretName
version: 1.0

[ClickHouse] SHALL support the `<secret_name>` subsection of the `<hashicorp_vault>`
section of secrets defined in the `<secrets>` section using the [HashiCorp vault]
backend to specify the name of the secret within the vault.

#### MultipleOptions

##### RQ.SRS-021.Secrets.Backend.MultipleOptions
version: 1.0

[ClickHouse] SHALL support simultaneous configuration of one or more options
for backend support of the `secrets` function at the same time using syntax
similar to the following:

```xml
<yandex>
    <secrets>
        <!-- plaintext secret, accessible by name `literal_password` -->
        <literal_password>this is the password</literal_password>

        <!-- server-key-derivation-based secret, accessible by name `server_key_32` -->
        <server_key_32>
            <secret_backend>server_key_provider</secret_backend>
            <server_key_provider>
                <key_bytes>32</key_bytes>
            </server_key_provider>
        </server_key_32>

        <!-- hashicorp-vault-managed secret, accessible by name `external_secret` -->
        <external_secret>
            <secret_backend>hashicorp_vault</secret_backend>
            <hashicorp_vault>
                <token>my-token</token>
                <namespace>my-namespace</namespace>
                <vault_host>my-vault-url</vault_host>
                <secret_name>my-secret-name</secret_name>
            </hashicorp_vault>
        </external_secret>
    </secrets>
</yandex>
```

#### Secret Backend

##### RQ.SRS-021.Secrets.Backend.SecretBackend
version: 1.0

[ClickHouse] SHALL support the `<secret_backend>` section as a subsection of
any secret to indicate which backend is provided for that secret.

```xml
<yandex>
    <secrets>
        <!-- The secret_backend tag indicates that this is a hashicorp vault -->
        <external_secret>
            <secret_backend>hashicorp_vault</secret_backend>
            <hashicorp_vault>
                <token>my-token</token>
                <namespace>my-namespace</namespace>
                <vault_host>my-vault-url</vault_host>
                <secret_name>my-secret-name</secret_name>
            </hashicorp_vault>
        </external_secret>
    </secrets>
</yandex>
```

## References

* **ClickHouse:** https://clickhouse.tech
* **GitLab Repository:** https://gitlab.com/altinity-qa/documents/qa-srs021-clickhouse-secrets-management
* **Revision History:** https://gitlab.com/altinity-qa/documents/qa-srs021-clickhouse-secrets-management

[ClickHouse]: https://clickhouse.tech
[GitHub]: https://github.com
[Git]: https://git-scm.com/
[GitLab Repository]: https://gitlab.com/altinity-qa/documents/qa-srs021-clickhouse-secrets-management
[HashiCorp vault]: https://www.vaultproject.io/
[Revision History]: https://gitlab.com/altinity-qa/documents/qa-srs021-clickhouse-secrets-management
''')
