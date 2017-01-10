# Axon
[![Axon](https://s29.postimg.org/sas2d3drb/axon128x128.png)](https://github.com/Eraden/axon)

[![CircleCI](https://circleci.com/gh/Eraden/axon/tree/master.svg?style=svg&circle-token=beb41b70d3092e2fd7d64a3989d1ea3de7bfe520)](https://circleci.com/gh/Eraden/axon/tree/master)
[![codecov](https://codecov.io/gh/Eraden/axon/branch/master/graph/badge.svg)](https://codecov.io/gh/Eraden/axon)
[![MIT licensed](https://img.shields.io/badge/license-MIT-blue.svg)](./License.md)

axon is database migration manager. It can create SQL migration files with timestamp (which you can edit),
execute them and skip every already executed file.

## Database management

Axon will create for you additional files:

* `conf/database.yml` - database connection data
* `db/order.yml` - describe which files should be executed and in which order
* `src/db/init.h` - header for database connection initialize
* `src/db/init.c` - source for parsing and perform database connection

After this you just need to add `db_init` to your application config file.
`axon` will print this information for you after `init`.

Additionally `axon` perform every migration in TRANSACTION and rollback everything if anything fails.
Failure database message will be printed out as well file path which causes problems.

```bash
axon db init
axon db create
axon db drop
axon db setup
axon db migrate
axon triggers
```

## Migrations:

`axon` can create SQL files for you but if you want more advance actions you need to edit them or create your own files.
They will not be overridden so you can safety change them.

```bash
axon db new table accounts id login pass last_logged:timestamp timestamps
axon db drop table accounts
axon db rename table accounts users
axon db new enum colors green yellow blue
axon db drop enum colors
axon db change accounts drop last_logged
axon db change accounts add age:int
axon db change accounts retype age:int
```

## Creator

Axon is delivered with `axon-creator` which handle file generating.

```bash
axon-creator new table accounts id login pass last_logged:timestamp timestamps
axon-creator drop table accounts
axon-creator rename table users accounts
axon-creator new enum colors green yellow blue
axon-creator drop enum colors
axon-creator change accounts drop last_logged
axon-creator change accounts add age:int
axon-creator change accounts retype login:text
```

## Migrator

Axon is delivered with `axon-migrator` which handle all psql connections and sql executions.

```bash
axon-migrator create
axon-migrator drop
axon-migrator setup
axon-migrator migrate
```

## Compiler

In version 1.1.1 introduce `axon-compiler` which compile migration callbacks.
Creator also was enhanced to create empty triggers files.

```bash
axon-compiler triggers
```

Example trigger file for migration with timestamp `1234`:

```c
#include <axon/triggers.h>
#include <axon/codes.h>

int before_1234_callback(AxonCallbackData *data);

int before_1234_callback(AxonCallbackData *data) {
  return AXON_SUCCESS;
}
```

## Types

`axon` support fallowing types of working environment:

* `dev`
* `prod`
* `test`

They can be set by creating `.flavor` ( [kore.io way](https://kore.io/) ) or by running command with environment variable:

```bash
echo 'test' >> .flavor
axon db create
```

```bash
KORE_ENV=test axon db create
```

## Files structure

`axon` will create some directories and files. Some of them are not in use right now but they will be in near future.
I highly recommend to exclude `.migrations` from version control system.

```asciidoc
conf
   ├── database.yml
   └── triggers.yml
src
  └── db
      ├── init.h
      └── init.c
db
 ├── order.yml
 ├── setup
 ├── seed
 └── migrate

.migrations
```

## `database.yml`

Example:

```yaml
dev:
  name: "kore_dev"
  port: 5432
  host: "localhost"
prod:
  name: "kore_prod"
  port: 5432
  host: "localhost"
test:
  name: "kore_test"
  port: 5432
  host: "localhost"
```

## `order.yml`

Example:

```yaml
seed:
  - first_seed_file.sql
  - second_seed_file.sql
setup:
  - first_setup.sql
  - second_setup.sql
```

## `triggers.yml`

Example:

```yaml
flags: -I./includes -L./lib
libs: libaxonutils libaxonconfig
```

All files should be stored in related directory. 
They can be stored in subdirectories but you need to add its name to file, ex. `subdirectory/file.sql`.
Do not pass absolute path, migrator is looking only for files in those directories.
Every not included file will be ignored as well as entry with non-existing files.

## Implementation

### Generating SQL

- [x] Creating new table
- [x] Creating drop table
- [x] Creating add column
- [x] Creating drop column
- [x] Creating new enum
- [x] Creating drop enum
- [x] Creating change type for column

### Execute

- [x] Create database
- [x] Drop database
- [x] Executing migration in transaction
- [x] Skip already executed files
- [x] Using setup `axon db setup` for setup database before `db migrate`

### TODO

- [ ] Support for `REFERENCES`
- [ ] Using seeds `axon db seed`
- [ ] Before migration code execution
- [ ] After migration code execution

## Pre-requirements

Ubuntu

```bash
sudo apt-get --yes install check
sudo apt-get --yes install clang-3.8
sudo apt-get --yes install libpq-dev postgresql-server-dev-all
sudo apt-get --yes install lcov
sudo apt-get --yes install libyaml-dev
sudo apt-get --yes install build-essential libunistring-dev
```

Mac OS

```bash
brew install libunistring
brew install postgresql
brew install libyaml
brew install lcov
```
