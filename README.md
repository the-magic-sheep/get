# get

This project is a work in progress and should not be considered stable.
Use at your own risk.

`get` is a basic command line utility to store file checkpoints that can later
be loaded.

It is basically a glorified undo button.

## Install

To install, you just need to run `install.sh`. You can do this using curl:

```bash
curl https://raw.githubusercontent.com/the-magic-sheep/get/master/install.sh | sh
```

The only dependency is gcc. It's also worth noting that the installer will
create a directory called `.get` in your home directory, and it will fail if
such a directory already exists. You can easily create a symlink to the executable:

```bash
sudo ln -s $HOME/.get/bin/get /bin/get
```

## Uninstall

`get` doesn't touch anything other than `~/.get` (except, of course, all of your files).

To uninstall, simply delete the `~/.get` directory.

## Usage

Running `get` with no commands will print the `get` help menu. Alternatively,

```bash
get -h
```

has the same effect.

For normal usage, the following commands are currently available:

### cp, checkpoint

Save a checkpoint of the target file with a checkpoint id.

#### Usage

```bash

get cp `<target-file>` `<checkpoint-name>`

# or

get checkpoint `<target-file>` `<checkpoint-name>`

```
### l, load

Load a saved checkpoint for a target file.

#### Usage

```bash

get l `<target-file>` `<checkpoint-name>`

# or

get load `<target-file>` `<checkpoint-name>`

```
### u, undo

Undo the previous load. Reverts the target file back to its state
before a checkpoint was loaded.

#### Usage

```bash

get u `<target-file>`

# or

get undo `<target-file>`

```

### d, drop

Delete a given checkpoint associated with the target file.

#### Usage

```bash

get d `<target-file>` `<checkpoint-name>`

# or

get drop `<target-file>` `<checkpoint-name>`

```

### list

List the saved checkpoints associated with the target file.

#### Usage

```bash

get list `<target-file>`

```

### purge

Delete all saved checkpoints associated with the target file.

#### Usage

```bash

get purge `<target-file>`

```
