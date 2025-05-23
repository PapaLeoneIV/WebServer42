#!/bin/bash

echo -e 'HTTP/1.1 200 OK\r\n'
echo -e 'Content-Type: text/html\r\n'
echo '<html>'
echo '<h3>'
echo 'Environment:'
echo '</h3>'
echo ''
echo '<pre>' 
echo '</pre>'
env
echo '<h3>'
echo 'Hostname'
echo '</h3>'
echo '<pre>' 
hostname
echo '</pre>'
echo '</html>'