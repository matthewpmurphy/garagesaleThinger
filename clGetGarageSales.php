<?php
/**
 * Created by PhpStorm.
 * User: gonzo_000
 * Date: 2/28/2015
 * Time: 4:30 PM
 */

include("inc/craigsList.inc");
ini_set('max_execution_time', 300);
$craigsList = new craigsList();
$craigsList->saveGarageSaleXmlToFile();