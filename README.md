# Axon
[![Axon](https://s29.postimg.org/sas2d3drb/axon128x128.png)](https://github.com/Eraden/axon)


[![CircleCI](https://circleci.com/gh/Eraden/axon/tree/master.svg?style=svg&circle-token=beb41b70d3092e2fd7d64a3989d1ea3de7bfe520)](https://circleci.com/gh/Eraden/axon/tree/master)
[![codecov](https://codecov.io/gh/Eraden/axon/branch/master/graph/badge.svg)](https://codecov.io/gh/Eraden/axon)


axon is database migration manager. It can create SQL migration files with timestamp (which you can edit),
execute them and skip every already executed file.

## Database management

Axon will create for you additional files:

* `conf/database.yml` - database connection data
* `src/db/init.h` - header for database connection initialize
* `src/db/init.c` - source for parsing and perform database connection

After this you just need to add `db_init` to your application config file.
`axon` will print this information for you after `init`.

Additionally `axon` perform every migration in TRANSACTION and rollback everything if anything fails.
Failure database message will be printed out as well file path which causes problems.

```bash
axon db init
axon db create
axon db migrate
axon db drop
```

## Migrations:

`axon` can create SQL files for you but if you want more advance actions you need to edit them or create your own files.
They will not be overridden so you can safety change them.

```bash
axon db new table accounts id login pass last_logged:timestamp timestamps
axon db change accounts drop last_logged
axon db change accounts add age:int
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
src
  └── db
      ├── init.h
      └── init.c
db
 ├── setup
 ├── seed
 └── migrate

.migrations
```

## Implementation

- [x] Creating new table SQL
- [x] Creating add column SQL
- [x] Creating drop column SQL
- [x] Executing migration in transaction
- [x] Create database
- [x] Drop database
- [x] Skip already executed files

Couple functionalities are missing right now, calling:

- [ ] Creating change type for column SQL
- [ ] Support for `REFERENCES`
- [ ] Using seeds `axon db seed`
- [ ] Using setup `axon db setup` for setup database before `db migrate`
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
