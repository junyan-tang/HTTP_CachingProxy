# HTTP Cache Proxy

## Overview of Implementation

- Use Boost to implement GET, POST, CONNECT requests
- Add caching mechanism based on RFC documents
- Revalidate cached response instead of re-requesting
- With great exception handling

## How to run it

Set up docker and run it directly: 
```Bash
sudo docker-compose up
```
