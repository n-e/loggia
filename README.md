# Loggia

Loggia is a fast web log analyzer that runs in the terminal. It doesn't need any setup or databases and is great for quickly getting useful information out of HTTP logs.

## Quick Start

    $ htdrill  -f log_file -l log_format rows cols filter

* `log_file`: The log file to analyze (e.g. : `/var/log/nginx/access.log`)
* `log_format`: The log format (e.g.: W3C, COMMON, COMBINED)
* `rows`: The field from the log to use as rows
* `cols`: The field from the log to use as columns
* `filter`: The field from the log to filter by (e.g. `browser_type<>Crawlers`)



**Example:** show the http version used by each browser:

    $ ./loggia -l VCOMBINEDNOP -f data/access.log.1 browser protocol 'browser_type<>Crawlers'
                          |HTTP/2.0 |HTTP/1.1 |HTTP/1.0 |(not set)
    ----------------------+---------+---------+---------+---------
    Mozilla/5.0           |         |     1321|        2|         
    Firefox/61.0          |      766|       35|         |         
    Safari/604.1          |      734|       15|         |         
    Firefox/57.0          |      362|         |       10|         
    Unknown               |        3|      152|       68|        4
    Firefox/52.0          |       96|       83|         |         
    Chrome/67.0.3396.99   |       83|       69|        1|         
    Chrome/59.0.3071.125  |      142|        3|         |         

## Installation

**Prerequisites:** A C compiler, that's all :-)

**Getting loggia:** Download it from the [releases page](https://github.com/n-e/loggia/releases)

**Build and installation:**

    make
    sudo cp loggia /usr/local/bin

## Recipes

### Check if your virtual hosts are configured properly

In a typical configuration you would want:

* Some virtual hosts to serve content
* Some virtual hosts to redirect to those that serve content
* The other virtual hosts to return a 404 error

To check that, run: 

    $ ./loggia -l VCOMBINEDNOP -f data/access.log.1 vhost status
                       |200 |404 |301 |304 |499 |400 |403 
    -------------------+----+----+----+----+----+----+----
    example.com        |1888| 415| 468|  40|  27|    |   3
    subd.example.com   |1281| 121|  41|    |   1|    |    
    old.example.com    |    | 129| 621|    |   1|    |    
    164.132.199.2      |    | 190|    |    |    |    |    
    _                  |    |   3|    |    |    |  11|    
    sfw.abcde.eu       |    |  12|    |    |    |    |    
    www.abcde.eu       |    |   7|    |    |    |    |    
    beta.abcde.eu      |    |   6|    |    |    |    |    
    img.abcde.eu       |    |   3|    |    |    |    |    
    pazuhinmaykl.ru    |    |   2|    |    |    |    |    
    bnp.3cx.eu         |    |   1|    |    |    |    |    
    localhost          |    |   1|    |    |    |    |    
    5.188.210.12       |    |   1|    |    |    |    |    
    2.ip-164-132-199.eu|    |   1|    |    |    |    |    

The first two virtual hosts return mainly HTTP 200 OK statuses, the old subdomain redirects, and the other virtual hosts return a 404 status. Everything's good.

 ## Supported fields

    agent           Full User Agent String
	  browser       Full Browser String (e.g.: Chrome/68.0.3440...)
	  browser_type  Browser type (e.g. Chrome, Crawlers...)
	date            Date
	host            Client Host (usually its IP)
	method          HTTP Method (GET, POST...)
	protocol        Protocol (HTTP/1.1...)
	referer         Referer
	req             Request (/plip/plop)
	status          Status Code (404...)
	time            Time
	vhost           Virtual Host
	url             vhost + req


 ## Command-line reference

    Usage: loggia -l log-format -f log-file [ rows ] [ cols ] [ filter ]


    -f <logfile>
    -l, --log-format=<logformat>
            Common formats: COMMON, VCOMMON, COMBINED, VCOMBINED
                            W3C, SQUID, CLOUDFRONT, AWSS3
    -d, --date-format=<dateformat>
    -t, --time-format=<timeformat>

    -h, --help                      This help.
    -V, --version                   Display version information and exit.

### Log, date and time format

If you use one of the common log file formats, only the -l flag is needed.

For custom file formats, loggia uses the same format as goaccess, you can [at the log format specifiers](https://goaccess.io/man#custom-log)  in their documentation for more details.