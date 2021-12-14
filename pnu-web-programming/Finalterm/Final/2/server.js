const express = require('express')
const app = express()
const maria = require('mysql')
const db_config = {
    host:'127.0.0.1',
    port:3306,
    user:'root',
    password:'',
    database:'201724603'
};
    
const conn = maria.createConnection(db_config);

//cors정책을 피하기 위한 모듈
const cors = require('cors')

// json 파일 받아서 사용하기 위한 모듈
var bodyParser = require('body-parser')

app.use(cors());
app.use(bodyParser.json())
app.use(bodyParser.urlencoded({extended:true}))

const port = 3000;

app.get('/', (req, res) => {
    res.send('Welcome My Server')
})

app.post('/create', function(req, res) {
  
    var name = req.body.name;
    var link = req.body.link;
    var image = req.body.image;
    var count = req.body.count;
    var category = req.body.category;

    conn.query('SELECT * FROM markTable WHERE name = "' + name + '"', function(error, result, fields) {
        if(error) {
            return res.json([{error: 'error', message: 'Fail to Load'}])
        } else {
            if(result.length >= 1) {
                return res.json([{error: 'error', message: '이미 있는 즐겨찾기 제목입니다. 다른 제목을 사용해주세요.'}])
            } else {
                conn.query(
                  'insert into markTable set name = "' +
                    name +
                    '", link = "' +
                    link +
                    '", category = "' +
                    category +
                    '", image = "' +
                    image +
                    '", count = "' +
                    count +
                    '"',
                  function (error, rows, fields) {
                    if (!error) {
                      console.log("insert Success.");
                      return res.json([{ error: "good" }]);
                    } else {
                      console.log(error);
                      return res.json([{ error: "error" }]);
                    }
                  }
                );
            }
        }
    })
})

app.post('/read', function(req, res) {
    var info = req.body.info;
    var pivot = req.body.pivot;
    if(info !== '' && pivot !== '') {
        conn.query("select * from markTable order by " + info + " " + pivot, function(error, rows, fields) {
            if(!error) {
                console.log("read Success.");
                res.json(rows)
            } else {
                console.log(error);
                res.json({ "err": error });
            }
        })
    } else {
        conn.query("select * from markTable", function(error, rows, fields) {
            if(!error) {
                console.log("read Success.");
                res.json(rows)
            } else {
                console.log(error);
                res.json({ "err": error });
            }
        })
    }
})

app.post('/delete', function(req, res) {
    var name = req.body.name;
    conn.query('delete from markTable where name ="' + name + '"', function(error, rows, fields) {
        if(!error) {
            console.log("delete Success.")
            return res.json([{ error: "good" }]);
        } else {
            console.log(error);
            return res.json([{ error: "error" }]);
        }
    })
})

app.post('/edit', function(req, res) {
    var name = req.body.name;
    var link = req.body.link;
    var image = req.body.image;
    var category = req.body.category;
    conn.query('update markTable set link = "' + link + '", image = "' + image + '", category = "' + category +
    '" where name = "' + name + '"', function(error, rows, fields) {
        if(!error) {
            console.log("edit Success.")
            return res.json([{ error: "good" }]);
        } else {
            console.log(error);
            return res.json([{ error: "error" }]);
        }
    })
})

app.listen(port, () => {
    console.log("Start Server" + port)
})