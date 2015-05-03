<?php
/*
 * Created by PhpStorm.
 * User: gonzo_000
 * Date: 2/21/2015
 * Time: 8:22 AM
 * Class to parse craigslist
 */

include("simple_html_dom.php");

class craigsList
{
    const CRAIGSLIST_URL = 'craigslist.org/';
    const CRAIGSLIST_URL_CITY = 'about/sites';
    const CRAIGSLIST_HOME = 'www.craigslist.org';
    const CRAIGSLIST_SITES_XML = 'cl-sites.xml';
    const CRAIGSLIST_LOG_FILE = 'cl.log';
    const CRAIGSLIST_GS_XML = 'cl-garagesales.xml';
    const CRAIGSLIST_XML_DIR = './xml/';

    public $link;
    public $title;
    public $description;

    public function craigsList()
    {

    }

    private function getCityList()
    {
        $url = 'http://' . self::CRAIGSLIST_HOME . '/' . self::CRAIGSLIST_URL_CITY;
        $web = file_get_contents($url);
        //$web = explode('<div id="map" style="border: 1px solid #551A8B; background-color: #71A4CD;"></div>', $web);
        $web = explode('<h1>', $web);
        //$web = explode('<footer>', $web[1]);
//        $html = str_get_html($web[0]);
//        foreach($html->find('a') as $element)
//        {
//            if(trim($element->href))
//                $city[] = $element->href;
//        }

        return $web[1];
    }

    private function generateCityXml()
    {
        $cities = $this->getCityList();
        $html = str_get_html($cities);
        foreach($html->find('h4') as $h4)
        {
            $state[] = $h4->innertext;
        }

        $i = 0;
        $xml = '<?xml version="1.0" encoding="UTF-8"?>';
        $xml .= "<craigslist>";
        $xml .= "<sites>";

        foreach($html->find('ul') as $ul)
        {
            foreach($ul->find('li') as $li)
            {
                foreach($li->find('a') as $a)
                {
                    if(trim($a->href))
                    {
                        $url = $this->getRssUrl($a->href);
                        $xml .= "<site>";
                        $xml .= "<url>" . $a->href . "</url>";
                        $xml .= "<area>" . $a->innertext . "</area>";
                        $xml .= "<state>" . $state[$i] . "</state>";
                        $xml .= "<rss>" . $url . "</rss>";
                        $xml .= "</site>";
                    }
                }
            }
            $i++;
        }
        $xml .= "</sites>";
        $xml .= "</craigslist>";
        return $xml;
    }

    public function saveCityXmlToFile()
    {
        $xml = $this->generateCityXml();
        if($xml)
        {
            file_put_contents(self::CRAIGSLIST_XML_DIR . self::CRAIGSLIST_SITES_XML, $xml);
        }
    }

    public function saveGarageSaleXmlToFile()
    {
        $xml = $this->getRssFeeds();
        if($xml)
        {
            file_put_contents(self::CRAIGSLIST_XML_DIR . self::CRAIGSLIST_GS_XML, $xml);
        }
    }

    private function readCityXml()
    {
        if(file_exists(self::CRAIGSLIST_SITES_XML))
        {
            $file = file_get_contents(self::CRAIGSLIST_XML_DIR . self::CRAIGSLIST_SITES_XML);
            $xml = simplexml_load_string($file);
            return $xml;
        }
    }

    private static function log($msg)
    {
        $path = self::CRAIGSLIST_LOG_FILE;
        $text = sprintf("%s || %s (%s) \n", date('c'), var_export($msg, TRUE), $_SERVER['REMOTE_ADDR']);
        file_put_contents($path, $text, FILE_APPEND | LOCK_EX);
    }

    private function getRssUrl($url)
    {
        if(!filter_var($url, FILTER_VALIDATE_URL) === false)
        {
            $str = parse_url($url);
            if($str['path'])
            {
                $url = $str['scheme'] . '://' . $str['host'] . '/search' . $str['path'] . '/gms?format=rss';
            }
            else
            {
                $url = $url . '/search/gms?format=rss';
            }
        }
        else
        {
            $url = 0;
        }
        return $url;
    }

    private function is_utf8($str)
    {
        return (bool) preg_match('//u', $str);
    }

    private function getRssFeeds()
    {
        $xml = $this->readCityXml();

        $myXml = '<CraigsList>';
        $myXml .= '<GarageSales>';
        foreach($xml->sites->site as $site)
        {
            $city = $site->area;
            $state = $site->state;

            if(!filter_var($site->rss, FILTER_VALIDATE_URL) === false)
            {
                $html = file_get_contents($site->rss);
                if(!$this->is_utf8($html))
                    $html = utf8_encode($html);

                $rss = simplexml_load_string($html);
                foreach($rss->item as $item)
                {
                    $myXml .= '<GarageSale>';
                    $myXml .= '<title>' . $item->title . '</title>';
                    $myXml .= '<url>' . $item->link . '</url>';
                    $myXml .= '<description><![CDATA[' . $item->description . ']]></description>';
                    $myXml .= '<city>' . $city . '</city>';
                    $myXml .= '<state>' . $state . '</state>';
                    $myXml .= '</GarageSale>';
                }
            }
            else
            {
                self::log('Not a valid URL: ' . $site->rss);
            }
        }
        $myXml .= '</GarageSales>';
        $myXml .= '</CraigsList>';
        return $myXml;
    }

    public function displaySale()
    {
        echo '<h2><a href="' . $this->link . '">' . $this->title . '</a></h2><p>' . $this->description . '</p>';
    }

    public function readGarageSalesXmlFile()
    {
        if(file_exists(self::CRAIGSLIST_GS_XML))
        {
            $file = file_get_contents(self::CRAIGSLIST_XML_DIR . self::CRAIGSLIST_GS_XML);
            $xml = simplexml_load_string($file);
            return $xml;
        }
    }
}