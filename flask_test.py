from datetime import datetime, timedelta
import json
import pytz
from flask import Flask, make_response

app = Flask(__name__)

@app.route('/')
def test_response():
    gmt_tz = pytz.timezone('GMT')
    current_time = datetime.now(gmt_tz)
    expire_time = current_time + timedelta(seconds=60)
    
    data = {'message': 'Hello, World!'}
    headers = {
        'Content-Type': 'application/json',
        # 'Cache-Control': 'max-age=15, must-revalidate, no-cache',
        'Cache-Control': 'public'
        # 'Expires': expire_time.strftime('%a, %d %b %Y %H:%M:%S %Z'),
        # 'ETag' : 'somerandomletters'
    }
    
    response = make_response(json.dumps(data))
    response.headers = headers
    
    return response
  
if __name__ == '__main__':
    app.run(debug=True, host='0.0.0.0')
