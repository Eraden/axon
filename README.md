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

~~Additionally `axon` perform every migration in TRANSACTION and rollback everything if anything fails.
Failure database message will be printed out as well file path which causes problems.~~

`axon` perform every migration in transaction but failing in middle of large bunch of migration files will reverse only last one.
This is cause by special execution process which is using `axon-requester` application.

You can perform migration in `TRANSACTION` only with flag `--skip-triggers` but none callback will be called of build.
This behaviour is aliased as `axan db migrate`.

```bash
axon db init
axon db create
axon db drop
axon db setup
axon db migrate
axon db setup
axon db seed
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

When you creating new table you can use some handy shortcuts:

### NOT NULL bang

Adding bang at end of column name will add `NOT NULL` constraint to column, for example:

`columnName!:columnType`

### REFERENCES

You can inform `axon` that column is actually reference to another table column by adding `(` and `)` with column name
between them, example:

`author_id:accounts(id)`

### UNIQUE NOT NULL id

All `id` columns are now `UNIQUE NOT NULL`! You don't need to pass `id!` to have `NOT NULL` constraint but if you do
this SQL still will be valid.

## Migrator

Axon is delivered with `axon-migrator` which handle all psql connections and sql executions.

```bash
axon-migrator create
axon-migrator drop
axon-migrator setup
axon-migrator migrate [--skip-triggers]
axon-migrator seed
```

## Compiler

Version 1.1.1 introduced `axon-compiler` which compile migration callbacks.
Creator also was enhanced to create empty triggers files.

```bash
axon-compiler triggers [--mem-check]
```

Example before trigger file for migration with timestamp `1234`:

```c
// ./db/1234_before_trigger.c
#include <axon/triggers.h>
#include <axon/codes.h>

int before_1234_callback(AxonCallbackData *data);

int before_1234_callback(AxonCallbackData *data) {
  return AXON_SUCCESS;
}
```

Compiler can link static or shared libraries to you triggers if you specify them in `conf/triggers.yml`.
Example for your static file called `libfoo.a` in directory `./vendor/lib/`.

```yaml
libs: foo
flags: -L./vendor/lib
```

New directory will be created called `.axon` in application directory and filled with object files (each for every source file).
Then compiler will link them together and create dynamic linked file `.axon/triggers.so`. This file will be loaded by `axon-requester`.

Triggers files have only tree restrictions:

* They must ends with `_callback.c`.
* Functions must match pattern `(before|after)_(\d+)_callback` and accept pointer to `AxonCallbackData`.
* When optional `--mem-check` flag is set `clang` must be installed and must be called `clang`.

You can store some data in `AxonCallbackData.payload` if you need.
This field was design to pass some data between before and after callback.
You can also pass function to clear this data using `AxonCallbackData.freePayload` field which accept `void (*fn)(void *)`.
Replacing payload in after callback without de-allocating previous data will cause memory leaks.

Example:

```c
// ./db/1234_before_trigger.c
#include <axon/triggers.h>
#include <axon/codes.h>

int before_1234_callback(AxonCallbackData *data);

// called after commit
void freePayload(void *payload) {
  printf(payload);
  free(payload);
}

int before_1234_callback(AxonCallbackData *data) {
  data->payload = calloc(sizeof(char), strlen("Database changed commited\n") + 1);
  data->freePayload = freePayload;
  return AXON_SUCCESS;
}
```

### Runner

```bash
axon-requester 1234 ./db/migarate/1234_create_table.sql
axon-requester 1234 --dry
```

Is special application which handle execution migration files.
Action perform by `axon-requester`:

* Load triggers library
* Load SQL if file was given
* Begin transaction
* Create `AxonCallbackData`
* Check if before trigger exists and run it with created `AxonCallbackData`
* Run SQL query with SQL stored in `AxonCallbackData`
* Check if after trigger exists and run it with existing `AxonCallbackData`
* Close transaction

Every step can `ROLLBACK` transaction and stop executing next.

`--dry` option execute steps with predefined SQL `CREATE LOCAL TEMPORARY TABLE IF NOT EXISTS dry_run_table ( id serial );`.
This mode should be used to track down memory leaks and crashes.

Database query will be performed regardless of mode unless SQL was removed from `AxonCallbackData`.

DO NOT FREE SQL! If you wish to change something in SQL clone it or completely replace.
`axon-requester` will take care of his and yours SQL (you don't need to create after callback just to remove your SQL).

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
- [x] Using seeds `axon db seed` for creating default or example table rows
- [x] Support PostgreSQL `REFERENCES` constraint
- [x] Support PostgreSQL `NOT NULL` constraint
- [x] Before migration code execution
- [x] After migration code execution

### TODO

### Known bugs

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
