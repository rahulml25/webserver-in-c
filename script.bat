@echo off
setlocal

set URL="http://localhost:3000"
set REQUEST_BODY="{\"key\": \"value\"}"

set COUNT=1
set MAX_REQUESTS=100

:send_request
curl -X POST %URL% -d %REQUEST_BODY%
set /a COUNT+=1
if %COUNT% leq %MAX_REQUESTS% goto send_request

endlocal
