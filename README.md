# koro

koro is database migration manager. It can create SQL migration files with timestamp (which you can edit),
execute them and skip every already executed file.

## Database management

Koro will create for you additional files:

* `conf/database.yml` - database connection data
* `src/db/init.h` - header for database connection initialize
* `src/db/init.c` - source for parsing and perform database connection

After this you just need to add `db_init` to your application config file.
`koro` will print this information for you after `init`.

Additionally `koro` perform every migration in TRANSACTION and rollback everything if anything fails.
Failure database message will be printed out as well file path which causes problems.

```bash
koro db init
koro db create
koro db migrate
koro db drop
```

## Migrations:

`koro` can create SQL files for you but if you want more advance actions you need to edit them or create your own files.
They will not be overridden so you can safety change them.

```bash
koro db new table accounts id login pass last_logged:timestamp timestamps
koro db change accounts drop last_logged
koro db change accounts add age:int
```

## Types

`koro` support fallowing types of working environment:

* `dev`
* `prod`
* `test`

They can be set by creating `.flavor` ( [kore.io way](https://kore.io/) ) or by running command with environment variable:

```bash
echo 'test' >> .flavor
koro db create
```

```bash
KORE_ENV=test koro db create
```

## Files structure

`koro` will create some directories and files. Some of them are not in use right now but they will be in near future.
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
- [ ] Using seeds `koro db seed`
- [ ] Using setup `koro db setup` for setup database before `db migrate`
- [ ] Before migration code execution
- [ ] After migration code execution
