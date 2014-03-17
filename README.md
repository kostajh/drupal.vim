Drupal Plugin for Vim
=====================

This is a plugin to facilitate Drupal development with Vim.

### Features

Partial list, work in progress:

- Drupal syntax highlighting for D6, D7 and D8
- Ctags generation with `drush vim-tag-gen`
- Drush integration (`:Drush`), asynchronous if
  [vim-dispatch](https://github.com/tpope/vim-dispatch) is installed

Installation
------------

Install using Vundle.

Post install you must run `ln -s ~/.vim/bundle/drupal.vim/vim.drush.inc
~/.drush/vim.drush.inc`.

Credits
-------

This is a fork of the [Vimrc project](http://drupal.org/project/vimrc) from
Drupal.org. The fork was created to move development forward and may merge back
into the Drupal.org plugin at some point. The commit history has been preserved, so you can see [those who
deserve credit](https://github.com/kostajh/drupal.vim/graphs/contributors) for this plugin, as well as where this has diverged from the Drupal.org project.

