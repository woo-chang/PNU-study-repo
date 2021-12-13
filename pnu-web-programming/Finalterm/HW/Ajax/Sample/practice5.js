const express = require('express')
const app = express()
const port = 3000

const maria = require('mysql')
const db_config = {
    host:'localhost',
    port:3306,
    user:'root',
    password:'123456',
    database:'202112345'
};

app.use(express.static('public'))
app.use(express.json())

app.get('/', (req, res) => {
  res.send('잘못된 요청입니다.')
})

app.get('/bookmarks', function (req, res) {
    const conn = maria.createConnection(db_config);
    conn.connect();
    conn.query('select * from bookmarks order by title asc', function(error, rows, fields) {
        if (!error) {
            res.json(rows);
        }
        else {
            console.log(error);
            res.json({ "err": error });
        }
    });
    conn.end();
})

app.post('/bookmarks', function (req, res) {  
    var title = req.body.title;
    var src = req.body.src;

    const conn = maria.createConnection(db_config);
    conn.connect();
    conn.query('insert into bookmarks set title = "' + title + '", link = "' + src + '"', function(error, rows, fields) {
        if (!error) {
            console.log("insert Success.");
        }
        else {
            console.log(error);
            res.json({ "err": error });
        }
    });
    conn.end();
})

app.delete('/bookmarks', function (req, res) {  
    var title = req.body.title;
    var sql = 'delete from bookmarks';
    if (title != undefined)
        sql += ' where title = "' + title + '"';

    const conn = maria.createConnection(db_config);
    conn.connect();
    conn.query(sql, function(error, rows, fields) {
        if (!error) {
            console.log("delete Success.");
        }
        else {
            console.log(error);
            res.json({ "err": error });
        }
    });
    conn.end();
})

app.listen(port, () => {
  console.log(`Example app listening at http://localhost:${port}`)
})