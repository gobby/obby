<?php

require_once './profiler.inc';

// The last slash is required
$GLOBALS['rewrite'] = '/obby_web/';

$GLOBALS['version'] = '0.3.0';

$GLOBALS['defrefresh'] = 60;
$GLOBALS['refresh'] = isset($_GET['refresh']) ? intval($_GET['refresh']) : NULL;

define('TOKEN_INDENTATION', 0);
define('TOKEN_IDENTIFIER', 1);
define('TOKEN_STRING', 2);
define('TOKEN_ASSIGNMENT', 3);
define('TOKEN_EXCLAMATION', 4);

function start_profile($function)
{
  if(isset($GLOBALS['prof']))
    $GLOBALS['prof']->startTimer($function);
}

function stop_profile($function)
{
  if(isset($GLOBALS['prof']))
    $GLOBALS['prof']->stopTimer($function);
}

// To be called like this:
//  make_link(array('file' => 'foo', 'document' => 'bar'));
function make_http_args($args_)
{
  if((NULL != $GLOBALS['refresh']) && !array_key_exists('refresh', $args_) )
    $args_['refresh'] = $GLOBALS['refresh'];

  $str = '';
  foreach($args_ as $arg=>$val)
    if($val !== NULL)
      $str .= '&amp;' . $arg . '=' . $val;
  return $str ? '?' . $str : ''; // $str == '' == false
}

function make_link($args_)
{
  if(isset($GLOBALS['rewrite']))
  {
    $str = $GLOBALS['rewrite'];
    if(isset($args_['file']) )
    {
      $str .= $args_['file'];
      if(isset($args_['document']) )
        $str .= '/' . $args_['document'];
    }
    $args_['file'] = NULL; $args_['document'] = NULL;
    return $str . make_http_args($args_);
  }
  else
    return basename(__FILE__) . make_http_args($args_);
}

class error
{
	function set_msg($err_msg_, $idx=NULL)
	{
		$this->m_err_msg = $err_msg_;
		$this->m_idx = $idx;
	}
	function _join_idx()
	{
	  if($this->m_idx === NULL) return '';
	  $str = ' (';
	  foreach($this->m_idx as $item) { $str .= $item . ', '; } 
	  return substr($str, 0, strlen($str) - 2) . ')'; 
  }
	function get_msg()
	{
		return $this->m_err_msg . $this->_join_idx();
	}

	function is_empty()
	{
	  // TODO: '' as initial error message?
	  return $this->m_err_msg == "No error message specified";
	}

	var $m_err_msg = "No error message specified";
	var $m_idx = NULL;
}

class object
{
  function object()
  {
  }

  function deserialise_attribute(&$tokenlist, &$token, &$error)
  {
    start_profile('object::deserialise_attribute');

    $name = $tokenlist[$token][1];
    if(parser::advance_required_token($tokenlist, $token, $error) == false)
      return false;

    if($tokenlist[$token][0] != TOKEN_ASSIGNMENT)
    {
      $error->set_msg("Expected '=' after attribute name");
      return false;
    }

    if(parser::advance_required_token($tokenlist, $token, $error) == false)
      return false;

    if($tokenlist[$token][0] != TOKEN_STRING)
    {
      $error->set_msg("Expected string as attribute value");
      return false;
    }

    $this->m_attributes[$name] = $tokenlist[$token][1];
    ++ $token;

    stop_profile('object::deserialise_attribute');
    return true;
  } 

  function deserialise(&$tokenlist, &$token, &$error, $indentation = 0)
  {
    start_profile('object::deserialise');
    //echo $tokenlist[$token][1] . "\n";
    $this->m_name = $tokenlist[$token][1];
    //print_r($this);
    //echo $this->m_name . "\n";

    ++ $token;

    // Following identifiers are attributes
    for(;;)
    {
      if($token == $tokenlist[0])
        break;

      if($tokenlist[$token][0] != TOKEN_IDENTIFIER)
        break;

      if($this->deserialise_attribute($tokenlist, $token, $error) == false)
        return false;
    }

    // Look for child objects
    for(;;)
    {
      //parser::advance_token($tokenlist, $token_h, $token_l);
      if($token == $tokenlist[0])
        break;

      if($tokenlist[$token][0] != TOKEN_INDENTATION)
      {
        $error->set_msg(sprintf('Expected child object instead of "%s"', $tokenlist[$token][1]) );
        return false;
      }

      $child_indentation = $tokenlist[$token][1];
      //$our_indentation = $indentation; //$this->get_indentation_level();

      // Belongs to some parent object
      if($child_indentation <= $indentation)
        break;

      if($child_indentation != $indentation + 1)
      {
        $error->set_msg("Child's indentation must be ours plus one");
        return false;
      }

      if(parser::advance_required_token($tokenlist, $token, $error) == false)
        return false;

      if($tokenlist[$token][0] != TOKEN_IDENTIFIER)
      {
        $error->set_msg("Expected child object after indentation");
        return false;
      }

      $child =& new object;
      if($child->deserialise($tokenlist, $token, $error, $child_indentation) == false)
        return false;
      $this->m_children[] =& $child;
    }
    
    stop_profile('object::deserialise');
    return true;
  }

  // Accessors
  function &get_name()            { return $this->m_name; }
  function &get_attribute($name_) { return $this->m_attributes[$name_]; }
  function &get_children()        { return $this->m_children; }

  //var $m_parent;
  var $m_name;
  var $m_attributes=array();
  var $m_children=array();
}

class parser
{
  /* This should probably be replaced by a filename. */
  function parser($fp_) { $this->m_fp = $fp_; }

  /* To be called to deserialse the passed file. */
  function deserialise(&$error)
  {
    start_profile('parser::deserialise');

    $tokenlist = array(0);
    $line_number = 0;
    while(false !== ($line = fgets($this->m_fp)))
    {
      $line = rtrim($line);
      if(false === parser::_tokenise($line_number ? "\n" . $line : $line, $tokenlist, $error) )
        return false;

      ++ $line_number;
    }

    $tokenlist[0] = count($tokenlist);
    $token = 1;

    if($tokenlist[$token][0] != TOKEN_EXCLAMATION)
    {
      $error->set_msg("Expected '!' at beginning of file");
      return false;
    }

    if(parser::advance_required_token($tokenlist, $token, $error) == false)
      return false;

    if($tokenlist[$token][0] != TOKEN_IDENTIFIER)
    {
      $error->set_msg("Expected identifier after '!'");
      return false;
    }

    $this->m_type = $tokenlist[$token][1];

    if(parser::advance_required_token($tokenlist, $token, $error) == false)
      return false;

    if($tokenlist[$token][0] != TOKEN_INDENTATION)
    {
      $error->set_msg("Expected newline after document type");
      return false;
    }

    if($tokenlist[$token][1] != 0)
    {
      $error->set_msg("Expected top-level object");
      return false;
    }

    if(parser::advance_required_token($tokenlist, $token, $error) == false)
    {
      $error->set_msg("Missing root object");
      return false;
    }

    $this->m_root = new object;
    if($this->m_root->deserialise($tokenlist, $token, $error) ==
       false)
      return false;

    // Must be EOF now
    if($token != $tokenlist[0])
    {
      $error->set_msg(
        sprintf('Expected end of input, got "%s"',
          $tokenlist[$token][1]) );
      return false;
    }

    stop_profile('parser::deserialise');
    return true;
  }

  // Throws an error if there is no next token.
  function advance_required_token(&$tokenlist, &$token, &$error)
  {
    start_profile('parser::advance_required_token');

    ++ $token;
    if($token == $tokenlist[0])
    {
      $error->set_msg("Unexpected end of input");
      return false;
    }

    stop_profile('parser::advance_required_token');
    return true;
  }

  function _tokenise($text, &$tokenlist, &$error)
  {
    start_profile('parser::_tokenise');

    $length = strlen($text);
    for ($i = 0; $i < $length;)
    {
      if ($text[$i] == "\n")
      {
        $i++;
        parser::_tokenise_indentation ($text, $i, $tokenlist);
      }
      elseif (ctype_alnum($text[$i]) || $text[$i] == '_')
      {
        parser::_tokenise_identifier ($text, $i, $tokenlist);
      }
      elseif ($text[$i] == "\"")
      {
        parser::_tokenise_string ($text, $i, $tokenlist);
      }
      elseif ($text[$i] == '=')
      {
        $tokenlist[] = array(TOKEN_ASSIGNMENT, '=');
        $i++;
      }
      elseif ($text[$i] == '!')
      {
        $tokenlist[] = array(TOKEN_EXCLAMATION, '!');
        $i++;
      }
      elseif ($text[$i] == ' ' || $text[$i] == "\t")
      {
        $i++;
      }
      else
      {
        $error->set_msg(sprintf('Unknown character found: \'%s\'', $text[$i]) );
        return false;
      }
    }

    stop_profile('parser::_tokenise');
    return true;
  }

  function _tokenise_indentation(&$text, &$pos, &$tokenlist)
  {
    start_profile('parser::_tokenise_indentation');
    for ($amount = 0; $pos < strlen($text) && $text[$pos] == ' '; ++ $pos)
      $amount++;

    if($pos < strlen($text) && $text[$pos] != "\n")
      $tokenlist[] = array(TOKEN_INDENTATION, $amount);

    stop_profile('parser::_tokenise_indentation');
  }

  function _tokenise_identifier(&$text, &$pos, &$tokenlist)
  {
    start_profile('parser::_tokenise_identifier');
    $pos_start = $pos;
    for (; $pos < strlen($text) && (ctype_alnum($text[$pos]) || $text[$pos] == '_'); ++ $pos)
      ;

    $tokenlist[] = array(TOKEN_IDENTIFIER, substr($text, $pos_start, $pos - $pos_start) );
    stop_profile('parser::_tokenise_identifier');
  }

  function _tokenise_string(&$text, &$pos, &$tokenlist)
  {
    start_profile('parser::_tokenise_string');
    $pos_start = ++ $pos;
    $escaped = false;

    for (; $pos < strlen($text) && (($escaped) || ($text[$pos] != "\"")); ++ $pos)
    {
      if ($escaped == true)
        $escaped = false;
      elseif ($text[$pos] == "\\")
        $escaped = true;
    }

    // TODO: Check for unexpected end of string

    $str = substr($text, $pos_start, $pos - $pos_start);
    parser::_tokenise_unescape($str);
    $tokenlist[] = array(TOKEN_STRING, $str);
    $pos++;
    stop_profile('parser::_tokenise_string');
  }

  function _tokenise_unescape(&$text)
  {
    $text = str_replace('\t', "\t", $text);
    $text = str_replace('\n', "\n", $text);
    $text = str_replace('\"', '"', $text);
  }

  function print_available_documents()
  {
    $children =& $this->m_root->get_children();
    $count = count($children);

    echo '<ul>';
    for($i = 0; $i < $count; ++ $i)
    {
      // TODO: Link
      if($children[$i]->get_name() == 'document')
      {
        $title = htmlentities($children[$i]->get_attribute('title') );
        echo '<li><a href="' . make_link(array('file' => $GLOBALS['_GET']['file'], 'document' => $title)) . '">' . $title . '</a></li>';
      }
    }
    echo '<li><a href="' . make_link(array('file' => $GLOBALS['_GET']['file'])) . '">Chat log</a></li>';
    echo '</ul>';
  }

  function print_chatlog(&$user_table)
  {
    $children =& $this->m_root->get_children();
    $count = count($children);
    
    $chat = NULL;
    for($i = 0; $i < $count; ++ $i)
    {
      if($children[$i]->get_name() == 'chat')
      {
        $chat =& $children[$i];
        break;
      }
    }

    if($chat == NULL) return;

    $messages =& $chat->get_children();
    $count = count($messages);

    echo '<pre class="chat">';
    for($i = 0; $i < $count; ++ $i)
    {
      $message =& $messages[$i];
      $text = $message->get_attribute('text');
      $time = intval($message->get_attribute('timestamp') );
      $class = NULL;

      if($message->get_name() == 'user_message')
      {
        $user = intval($message->get_attribute('user') );
        $text = '&lt;' . $user_table->get_name($user) . '&gt; ' . $text;
        $class = 'chat_user';
      }
      else if($message->get_name() == 'system_message')
        $class = 'chat_system';
      else if($message->get_name() == 'server_message')
        $class = 'chat_server';
      else
        continue;

      $timestr = date('r', $time);
      echo '<span class="' . $class . '">[' . $timestr . '] ' . $text . '</span>' . "\n";
    }
    echo '</pre>';
  }

  function& get_user_table(&$error)
  {
    $children =& $this->m_root->get_children();
    $count = count($children);

    $user_table = NULL;
    for($i = 0; $i < $count; ++ $i)
    {
      if($children[$i]->get_name() == 'user_table')
      {
        $user_table =& $children[$i];
        break;
      }
    }

    if(NULL === $user_table)
    {
      $error->set_msg('Session has no user table');
      return false;
    }

    return new user_table($user_table);
  }

  function print_document($document, &$error)
  {
    // Get document
    $children = $this->m_root->get_children();
    $count = count($children);

    for($i = 0; $i < $count; ++ $i)
      if($children[$i]->get_name() == 'document')
        if($children[$i]->get_attribute('title') == $document)
          $docobj = $children[$i];

    if(!$docobj)
    {
      $error->set_msg('Requested document not found');
      return false;
    }

    $children = $docobj->get_children();
    $count = count($children);

    if($count == 0)
    {
      $error->set_msg('Document has not been loaded');
      return false;
    }

    echo "<pre class=\"document\">\n";
    for($i = 0; $i < $count; ++ $i)
    {
      // Only process lines (TODO: omit this check because all children have to be lines?
      if($children[$i]->get_name() == 'line')
      {
        // Iterate through parts
        $parts = $children[$i]->get_children();
        $part_count = count($parts);
        for($p = 0; $p < $part_count; ++ $p)
        {
          // Get author
          $user = intval($parts[$p]->get_attribute('author') );
          // Get content
          $content = htmlentities($parts[$p]->get_attribute('content') );
          echo '<span class="user_' . $user . '">' . $content . '</span>';
        }

        echo "\n";
      }
    }

    echo "</pre>\n";
    return true;
  }

  var $m_fp = NULL;
  var $m_type = "Unknown type";
  var $m_root = NULL;
}

class user_table
{
  function user_table(&$obj)
  {
    $users =& $obj->get_children();
    $count = count($users);

    for($i = 0; $i < $count; ++ $i)
    {
      $user  =& $users[$i];
      $id    =& intval($user->get_attribute('id') );
      $name  =& htmlentities($user->get_attribute('name') );
      $color =& sprintf('%x', intval($user->get_attribute('colour'), 16));
      $this->m_users[$id] = array($name, $color);
    }
  }

  function& get_name($id)
  {
    return $this->m_users[$id][0];
  }

  function& get_color($id)
  {
    return $this->m_users[$id][1];
  }

  function print_css()
  {
    foreach($this->m_users as $id => $user)
    {
      //echo "pre.document span.user_$id, ul.user_list li.user_$id {
      echo ".user_$id {
        color: black;
        background: #$user[1];
      }";
    }
    
    //echo "pre.document span.user_0, ul.user_list li.user_0 {
    echo ".user_0 {
      color: black;
      background: #fff;
    }";
  }

  function print_list()
  {
    echo '<ul class="user_list">';
    foreach($this->m_users as $id => $user)
    {
      echo '<li class="user_' . $id . '">'. $user[0] . '</li>';
    }
    echo '</ul>';
  }

  var $m_users = array();
}

function microtime_float()
{
   list($usec, $sec) = explode(" ", microtime());
   return ((float)$usec + (float)$sec);
}

$error = new error;

if(php_sapi_name() == "cli")
{
  $filename = 'stdin';

  /* Use stdin when called from CLI */
  $fp = fopen('php://stdin', 'r');
  $pub_date = time();
}
else
{
  /* Restrict file access to the path of the script and below...
     This is a temporary measure until I find a nice replacement for
     addslashes which prevents double dot attacks. */
  ini_set('open_basedir', getcwd());
  $filename = isset($_GET['file']) ? addslashes($_GET['file']) : NULL;
  if(NULL == $filename)
    $error->set_msg('No file specified, bailing out!');
  else if(!file_exists($filename))
    $error->set_msg('File does not exist, bailing out!');
  else if(!($fp = fopen(getcwd() . '/' . $filename, 'r')) )
    $error->set_msg('Could not open file ' . getcwd() . '/' . $filename . ', bailing out!');
  else
    $pub_date = filemtime($filename);
}

$pub_date = gmstrftime("%Y-%m-%dT%H:%M:%s+00:00", $pub_date);

if($fp)
{
  $parser = new parser($fp);
  if($parser->deserialise($error) )
    $user_table =& $parser->get_user_table($error);
}

$document = isset($_GET['document']) ? $_GET['document'] : NULL;
$refresh = isset($_GET['refresh']) ? $_GET['refresh'] : NULL;

echo '<?xml version="1.0" encoding="UTF-8" ?>';
?>

<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.1//EN" "http://www.w3.org/TR/xhtml11/DTD/xhtml11.dtd">
<html xmlns="http://www.w3.org/1999/xhtml" xml:lang="en">
 <head>
  <title><?php echo 'obby.php' . ($error->is_empty() ? (' - ' . htmlentities($filename) . ($document ? (' (' . htmlentities($document) . ')') : '')) : '') ?></title>
  <meta http-equiv="Content-Type" content="text/html; charset=UTF-8" />
  <?php if(NULL !== $GLOBALS['refresh']) { ?>
  <meta http-equiv="Refresh" content="<?php echo $GLOBALS['refresh']; ?>;
     URL=<?php echo make_link(array('file' => $filename, 'document' => $document)); ?>" />
  <?php } ?>
  <meta name="date" content="<?php echo $pub_date; ?>" />
  <meta name="generator" content="obby.php (<?php echo $GLOBALS['version']; ?>)" />

  <style type="text/css">
   body {
	   font: small "Lucida Grande", "Trebuchet MS", "Helvetica", "Verdana", "Arial", sans-serif;
	   background: #fff;
	   color: #000;
   }

   pre.document, pre.chat {
   	margin-right: 10em;
   }

   .user_list {
     position: absolute;
     top: 3.5em;
     left: auto;
     right: 2em;
   }

   .menu {
     position: absolute;
     top: 1em;
     left: auto;
     right: 2em;
   }

   body > .user_list {
   	 position: fixed; /* ph34r */
   }

   body > .menu {
     position: fixed;
   }

   .user_list li {
     padding: 0.2em 0.3em;
     margin: 3px 0;
     list-style: square;
     border: 1px solid #333;
   }

   span.chat_user {
     color: black;
     background: white;
   }

   span.chat_system {
     color: blue;
     background: white;
   }

   span.chat_server {
     color: #228b22; /* forest green */
     background: white;
   }

   <?php
     if($error->is_empty() )
       $user_table->print_css();
   ?>
  </style>
 </head>

 <body>
  <?php if(!$error->is_empty() ) {
    echo '<p class="error">Error: ' . $error->get_msg() . '</p>';
  } else {

    echo '<div class="menu">';
    if(NULL != $GLOBALS['refresh'])
      echo '[ <a href="' . make_link(array('file' => $filename, 'document' => $document, 'refresh' => NULL)) . '">Do not refresh</a> ]';
    else
      echo '[ <a href="' . make_link(array('file' => $filename, 'document' => $document, 'refresh' => $GLOBALS['defrefresh'])) . '">Refresh every ' . $GLOBALS['defrefresh'] . ' seconds</a> ]';
    echo '</div>';

    $user_table->print_list();

    echo '<h1>Available documents</h1>';
    $parser->print_available_documents();

    if($document != NULL) {
      echo '<h2>' . $document . '</h2>';
      $parser->print_document($document, $error);

      if(!$error->is_empty() )
        echo '<p class="error">Error: ' . $error->get_msg() . '</p>';
    } else {
      echo '<h2>Chat log</h2>';
      $parser->print_chatlog($user_table);
    }
  } ?>
 </body>
</html>
<?php
/*$GLOBALS['prof'] = new Profiler(true, true);
$GLOBALS['prof']->startTimer('outer');
error_reporting(E_ALL);
$start = microtime_float();
$list = array();
$first = false;
$parser = new parser($fp);
var_dump($parser->deserialise($error));
var_dump($error->get_msg());
//if(!$parser->deserialise($error) )
//  die($error->get_msg() );
//print_r($parser);
fclose($fp);
var_dump(microtime_float() - $start);
$GLOBALS['prof']->stopTimer('outer');
$GLOBALS['prof']->printTimers(true);
*/
?>
