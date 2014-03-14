<?php

/**
 * @file
 *   Drush tools to support the drupal.vim project.
 */

/**
 * Implements hook_drush_command().
 */
function vim_drush_command() {
  $items = array();

  $items['vim-tag-gen'] = array(
    'description' => 'Generate a tags file for use by vim and other programs.',
    'arguments' => array(
      'path' => 'System path to a file or directory (e.g. your Drupal distribution or a module folder). Defaults to the current directory.',
    ),
    'options' => array(
      'tag-file' => 'Name of the output file (defaults to "tags")',
      'ctags' => 'Path to the exuberant ctags executable (defaults to "ctags")',
      'include-js' => 'Include tags for .js files (defaults to "no")',
      'include-test' => 'Include tags for .test files (defaults to "no")',
      'make-portable' => 'Replace path with \'$DRUPAL_ROOT\' (defaults to "no")',
    ),
    'examples' => array(
      'drush vim-tag-gen $HOME/Sites/drupal-7 --tag-file=drupal7.tags'
      => 'Generate a tags file for your distribution of Drupal 7 and save it as drupal7.tags in the current directory.',
      'drush tags sites/all/modules/ctools --ctags=/usr/local/bin/ctags'
      => 'Generate a tags file for the ctools module using a specific ctags executable and save it as tags in the current directory.',
    ),
    'aliases' => array('tags'),
    'bootstrap' => DRUSH_BOOTSTRAP_DRUSH,
  );

  return $items;
}

/**
 * Implements drush_hook_COMMAND_validate().
 */
function drush_vim_tag_gen_validate() {
  // Bail out if ctags is not executable or is not the right version.
  $ctags = drush_get_option('ctags', 'ctags');
  $ctags = escapeshellcmd($ctags);
  drush_set_option('ctags', $ctags);
  exec("$ctags --version", $output, $retval);
  if ($retval || !preg_match('/^Exuberant Ctags/', $output[0])) {
    return drush_set_error('DRUSH_APPLICATION_ERROR', dt('Make sure that Exuberant Ctags is installed (http://ctags.sourceforge.net/) and specify the path with (for example) --ctags=/usr/local/bin/ctags'));
  }
  // Check that the tags file is writable.
  $tag_file = drush_get_option('tag-file', 'tags');
  touch($tag_file);
  $tag_file = realpath($tag_file);
  drush_set_option('tag-file', $tag_file);
  if (!is_writable($tag_file)) {
    return drush_set_error('DRUSH_PERM_ERROR', dt('Cannot write the tags file. Specify another path with --tag-file=path/to/tags'));
  }
  // Check that the boolean flags are "yes" or "no".
  $flags = array('include-js', 'include-test', 'make-portable');
  foreach ($flags as $flag) {
    $option = drush_get_option($flag, 'no');
    if ($option != 'yes' && $option != 'no') {
      // TODO: Find a better error code.
      return drush_set_error('DRUSH_UNKNOWN_OPTION', dt('The --include-* options should be specified as "yes" or "no".'));
    }
  }
}

/**
 * Implements drush_hook_COMMAND().
 */
function drush_vim_tag_gen($path = '.') {
  // Get the option values.  The validate function sets some of them.
  $ctags = drush_get_option('ctags', 'ctags');
  $tag_file = drush_get_option('tag-file', 'tags');
  $include_js = drush_get_option('include-js', 'no');
  $include_test = drush_get_option('include-test', 'no');
  $portable = (drush_get_option('make-portable', 'no') == 'yes');
  if ($portable) {
    $path = realpath($path);
    // Replace the path (but not any trailing '/') with '$DRUPAL_ROOT'.
    $shortpath = rtrim($path, '/');
    $length = strlen($shortpath);
  }

  // Figure out which ctags options to use.
  $excludes = array('.git');
  $langmap = 'php:+.module.install.inc.profile.theme.engine';
  if ($include_test == 'yes') {
    $langmap .= '.test';
  }
  else {
    $excludes += array('simpletest', 'tests', 'Tests', 'Test', 'Test.php');
  }
  if ($include_js != 'yes') {
    $langmap .= ',javascript:';
  }
  $options = "-R --totals --extra=+f --langmap=$langmap --php-kinds=-v";
  $options .= ' --exclude=' . implode(' --exclude=', $excludes);

  // Run ctags, generating $tag_file.
  print dt('Some warning messages from ctags are normal.') . "\n\n";
  exec("$ctags -f $tag_file $options $path");

  // Skip lines matching these patterns.
  $patterns = array();
  // Skip anything that looks like a comment.
  $patterns[] = '#/\^\s*(\\\\/\\\\/|/\*|\*|\'\')#';
  // Skip anything that looks like 'key' => 'value'.
  $patterns[] = '/[\'"]#?\w+[\'"]\s*=>/';
  // Skip anything that looks like $output .= 'lotsa text'.
  $patterns[] = '#/\^\s*\$\w+\s*\.?\=#';

  // Read the file into an array.
  $lines = file($tag_file);
  $handle = fopen($tag_file, 'w');
  // Process the lines and rewrite the file.
  foreach ($lines as $line) {
    foreach ($patterns as $pattern) {
      if (preg_match($pattern, $line) == 1) {
        continue;
      }
    }
    if ($portable) {
      $line = str_replace($shortpath, '$DRUPAL_ROOT', $line);
    }
    fwrite($handle, $line);
  }
  fclose($handle);
}
