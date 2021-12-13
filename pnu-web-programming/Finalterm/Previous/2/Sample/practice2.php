<?php
    extract($_POST);
    $existFlag = FALSE;
    $mysqli = new mysqli("127.0.0.1", "root", "980808", "201724483");
    if($mysqli->connect_errno){
        die("Failed to connect to MySQL");
    }

    $post_data = json_decode(file_get_contents('php://input'));

    if($post_data->func == 'add'){
        if($result = $mysqli->query("select * from bookinfo where title = '$post_data->title'")){
            if($result->num_rows != 0){

                $existFlag = TRUE;
            }
            else{
                if($mysqli->query("insert into bookinfo(title,author,translator,image,publisher,classification,year,month,page,price) values(
                    '$post_data->title','$post_data->author','$post_data->translator','$post_data->image','$post_data->publisher','$post_data->classification',
                    $post_data->year,$post_data->month,$post_data->page,$post_data->price)") !== TRUE){
                    die("Failed to Insert Data");
                }
            }
        }
    
        header("Content-Type: application/json");
        echo(json_encode(array("existFlag" => $existFlag)));
    }


    if($post_data->func == 'showBooks'){
        // 데이터베이스의 여러 행 데이터를 넘기려면 각 column마다 array를 만들고 column을 key로 하여금 만든 array를 value로 넘겨주자.
        $title = array();
        $author = array();
        $classification = array();
        $price = array();
        
        if($bookArr = $mysqli->query("select * from bookinfo order by $post_data->sortBy $post_data->sortOrder")){
            while($data = mysqli_fetch_array($bookArr)){
                array_push($title,$data['title']);                
                array_push($author,$data['author']);                     
                array_push($classification,$data['classification']);   
                array_push($price,$data['price']);                
            }
            header("Content-Type: application/json");
            echo(json_encode(array("title" => $title, "author" => $author, "classification" => $classification, "price" => $price)));
        }

    }

    if($post_data->func == 'getBookInfo'){
        /*$title;
        $author;
        $translator;
        $image;
        $publisher;
        $classification;
        $year;
        $month;
        $page;
        $price;*/
        if($bookArr = $mysqli->query("select * from bookinfo where title = '$post_data->title'")){
            while($data = mysqli_fetch_array($bookArr)){
                $title = $data['title'];
                $author = $data['author'];
                $translator = $data['translator'];
                $image = $data['image'];
                $publisher = $data['publisher'];
                $classification = $data['classification'];
                $year = $data['year'];
                $month = $data['month'];
                $page = $data['page'];
                $price = $data['price'];
            }
            header("Content-Type: application/json");
            echo(json_encode(array("title" => $title, "author" => $author, "translator" => $translator, "image" => $image, 
            "publisher" => $publisher, "classification" => $classification, "year" => $year, "month" => $month, 
            "page" => $page, "price" => $price)));
        }
    }

    if($post_data->func == 'getRowsNum'){
        if($allLinks = $mysqli->query("select * from linkInfo")){
            $rowNum = $allLinks->num_rows;
            header("Content-Type: application/json");
            echo(json_encode(array("rowNum" => $rowNum)));
        }
    }
?>