get is a basic command line utility to store file checkpoints that can later be loaded. 

It is basically a glorified undo button.

To install, you just need to run `install.sh`.

The only dependency is gcc. It's also worth noting that the installer will
create a directory called `.get` in your home directory, and it will fail if 
such a directory already exists. You can easily create a symlink to the executable:

```bash
sudo ln -s $HOME/.get/bin/get /bin/get
```
