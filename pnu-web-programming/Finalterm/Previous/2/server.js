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
    var cover = req.body.cover;
    var title = req.body.title;
    var author = req.body.author;
    var translator = req.body.translator;
    var publish = req.body.publish;
    var category = req.body.category;
    var year = req.body.year;
    var month = req.body.month;
    var page = req.body.page;
    var price = req.body.price;
    const conn = maria.createConnection(db_config);
    conn.connect();
    conn.query('insert into bookTestTable set cover = "' + cover + '", title = "' + title + '", author = "' + author +
    '", translator = "' + translator + '", publish = "' + publish + '", category = "' + category + '", year = "' + year +
    '", month = "' + month + '", page = "' + page + '", price = "' + price + '"', function(error, rows, fields) {
        if (!error) {
            console.log("insert Success.");
        }
        else {
            console.log(error);
            res.json({ "err": error });
        }
    });
    conn.end()
})

app.post('/read', function(req, res) {
    var info = req.body.info;
    var pivot = req.body.pivot;
    const conn = maria.createConnection(db_config);
    conn.connect();
    if(info !== '' && pivot !== '') {
        conn.query("select title, author, category, price from bookTestTable order by " + info + " " + pivot, function(error, rows, fields) {
            if(!error) {
                console.log("read Success.");
                res.json(rows)
            } else {
                console.log(error);
                res.json({ "err": error });
            }
        })
    } else {
        conn.query("select title, author, category, price from bookTestTable", function(error, rows, fields) {
            if(!error) {
                console.log("read Success.");
                res.json(rows)
            } else {
                console.log(error);
                res.json({ "err": error });
            }
        })
    }
    conn.end();
})

app.post('/readDetail', function(req, res) {
    var title = req.body.title;
    var author = req.body.author;
    const conn = maria.createConnection(db_config);
    conn.connect();
    conn.query('select * from bookTestTable where title ="' + title + '" AND author ="' + author + '"', function(error, rows, fields) {
        if(!error) {
            res.json(rows)
        } else {
            console.log(error);
            res.json({ "err": error });
        }
    })
    conn.end();
})

app.listen(port, () => {
    console.log("Start Server" + port)
})