<?php
    ini_set('display_errors', 1);   // 에러 메시지 출력 On

    // Connect to MySQL
    $mysqli = new mysqli("localhost", "root", "비밀번호", "202112345");
    if ( $mysqli->connect_errno )
        die( "<p>Could not connect to database</p>" );

    $mode = "select";
    $curPage = 0;
    if(isset($_POST['mode']))
        $mode = $_POST['mode'];
    
    if(isset($_POST['curPage']))
        $curPage = $_POST['curPage'];
    $totalCount = 0;
    $err = "";

    if ($mode == "insert")
    {
        // 중복 검사 먼저 수행
        $sql = "select * from bookmarks where title='".$_POST['linkTitle']."'";
        if ( $result = $mysqli->query($sql) ) 
        {
            if ($result->num_rows > 0)
            {
                $err = "이미 있는 즐겨찾기 제목입니다. 다른 제목을 사용해주세요.";
            }
            else
            {
                $sql = "insert into bookmarks set title = '".$_POST['linkTitle']."', link = '".$_POST['linkSrc']."'";
                $mysqli->query($sql);
            }
            $result->close();
        }
    }
    else if ($mode == "delete")
    {
        // 존재 여부 검사 먼저 수행
        $sql = "select * from bookmarks where title='".$_POST['linkTitle']."'";
        if ( $result = $mysqli->query($sql) ) 
        {
            if ($result->num_rows == 0)
            {
                $err = "일치하는 제목을 가진 즐겨찾기가 없습니다.";                
            }
            else
            {
                $sql = "delete from bookmarks where title = '".$_POST['linkTitle']."'";
                $mysqli->query($sql);
            }
            $result->close();
        }
    }
    else if ($mode == "delete_all")
    {
        $sql = "delete from bookmarks";
        $mysqli->query($sql);
    }
    
    $select_sql = "select * from bookmarks order by title asc";
    $data_arr = array();
    $data_cnt = 0;
    if ( $result = $mysqli->query($select_sql) ) 
    {
        $totalCount = $result->num_rows;
    }
?>
<!DOCTYPE html>
<html lang="ko">
<head>
    <meta charset="UTF-8">
    <title>Example</title>
    <style type="text/css">
        #linkContainer {
            display: flex;
            flex-wrap: wrap;
            width: 310px;
        }
    
        #linkContainer a  {
            display: block;
            list-style-type: none;
            text-align: center;
            font-size: 3em;
            border: 1px solid;
            width: 150px;
            float: left;
            margin: 1px;
        }        
        a   { 
            text-decoration: none;
        }
        .pagination {
            visibility: hidden;
            display: inline-block;
        }
        .pagination div {
            float: left;
            padding: 8px 16px;
        }
        .pagination div.active {
            background-color: #4CAF50;
            color: white;
        }
        .pagination div:hover:not(.active) {background-color: #ddd;}
        .maindiv {
            width: 320px;
            float: left;
        }
        iframe {
            width: 500px;
            height: 500px;
            float: left;
        }
    </style>
    <script type="text/javascript">
        //var linkArr = new Array();
        //var curPage = 0;

        function addLink()
        {
            //if (linkArr.findIndex(checkTitle) >= 0)
            //{
            //    window.alert("이미 있는 즐겨찾기 제목입니다. 다른 제목을 사용해주세요.");
            //    return;
            //}
            //linkArr.push({title: document.forms[0].linkTitle.value, src: document.forms[0].linkSrc.value});
            //linkArr.sort(compareTitle);
            if (document.forms[0].linkTitle.value == "" || document.forms[0].linkSrc.value == "")
            {
                window.alert("추가할 즐겨찾기의 제목과 링크를 모두 입력해주세요.");
                return;
            }

            document.forms[0].mode.value = "insert";

            //document.forms[0].reset();
            //makePages();
            //showLinks();
            //updateData();
            document.forms[0].submit();
        }
        /*function checkTitle(link)
        {
            return link.title == document.forms[0].linkTitle.value;
        }
        function compareTitle(link1, link2)
        {
            if (link1.title > link2.title)
                return 1;
            else if (link1.title < link2.title)
                return -1;
            else
                return 0;
        }*/   
        function deleteLink()
        {
            //var idx = linkArr.findIndex(checkTitle);
            //if (idx < 0)
            //{
            //    window.alert("일치하는 제목을 가진 즐겨찾기가 없습니다.");
            //    return;
            //}
            //linkArr.splice(idx, 1);
            //makePages();
            //showLinks();
            //updateData();
            if (document.forms[0].linkTitle.value == "")
            {
                window.alert("제거할 즐겨찾기의 제목을 입력해주세요.");
                return;
            }
            document.forms[0].mode.value = "delete";
            document.forms[0].submit();
        }
        function clearAll()
        {
            //linkArr = Array();
            //document.getElementById("linkContainer").innerHTML = "";
            //updateData();
            document.forms[0].mode.value = "delete_all";
            document.forms[0].submit();
        }
        /*function showLinks()
        {
            var linkContainer = document.getElementById("linkContainer");
            linkContainer.innerHTML = "";

            var startIdx = curPage * 10;
            var endIdx = startIdx + 9;
            for(var idx = startIdx; idx <= endIdx && idx < linkArr.length; idx++)
            {
                var link = makeLink(linkArr[idx].title, linkArr[idx].src);
                linkContainer.innerHTML += link;
            }
        }
        function makeLink(title, src)
        {
            return "<a href='" + src + "' target=\"linkFrame\">" + title + "</a>";
        }*/
        function makePages()
        {
            var curPage = document.forms[0].curPage.value;
            if (<?php echo $totalCount;?> > 10)
            {
                var pageNav = document.getElementById("pageNav");
                pageNav.style.visibility = "visible";
                pageNav.innerHTML = "";
                var nPage = Math.ceil(<?php echo $totalCount;?> / 10);
                for(var i =0; i < nPage; i++)
                {
                    if (i == curPage)
                        pageNav.innerHTML += "<div class=\"active\" onclick=\"changePage(" + i +")\">" + (i + 1) + "</div>";
                    else
                        pageNav.innerHTML += "<div onclick=\"changePage(" + i +")\">" + (i + 1) + "</div>";

                }
            }
        }
        function changePage(pageNum)
        {
            //curPage = pageNum;
            document.forms[0].curPage.value = pageNum;
            document.forms[0].mode.value = "select";
            makePages();
            //showLinks();
            document.forms[0].submit();
        }
        /*function updateData()
        {
            localStorage.removeItem("links");
            localStorage.setItem("links", JSON.stringify(linkArr));
        }*/
        function makeAlert()
        {
            var err = "<?php echo $err;?>";
            if (err.length > 0)
            {
                window.alert(err);
                return;
            }
        }
        function start()
        {
            makeAlert();
            //var links = localStorage.getItem("links");
            //if (links != null)
            //{
            //    linkArr = JSON.parse(links);
                makePages();
                //showLinks();
            //}
        }
    </script>
    </head>
<body onload="start()">
    <div class="maindiv">
        <form name="form1" method="POST" action="Q2.php">
            <label>즐겨찾기 제목: <input type="text" name="linkTitle" id="linkTitle"></label><br>
            <label>즐겨찾기 링크: <input type="text" name="linkSrc" id="linkSrc"></label><br>
            <input type="button" name="addBtn" id="addBtn" value="즐겨찾기 추가" onclick="addLink()">
            <input type="button" name="delBtn" id="delBtn" value="즐겨찾기 삭제" onclick="deleteLink()">
            <input type="reset" name="ClearBtn" id="clearBtn" value="모두 삭제" onclick="clearAll()">
            <input type="hidden" name="mode" id="mode" value="select"/>
            <input type="hidden" name="curPage" id="curPage" value="<?php echo $curPage;?>"/>
        </form>
        <br>
        <div id="linkContainer">
            <?php
                if ($result)
                {
                    while($data = $result->fetch_assoc())
                    {
                        $data_arr[$data_cnt++] = array("title"=>$data['title'], "link"=>$data['link']);
                    }
                    $result->close();
                }
                    
                $startIdx = $curPage * 10;
                $endIdx = $startIdx + 9;
                for ($i = $startIdx; $i <= $endIdx && $i < $data_cnt; $i++)
                {
                    echo "<a href='".$data_arr[$i]['link']."' target=\"linkFrame\">".$data_arr[$i]['title']."</a>";
                }                    
            ?>
        </div>
        <nav class="pagination" id="pageNav">
        </nav>
    </div>
    <iframe title="iframe for link" name="linkFrame"></iframe>
</body>
</html>
<?php
    $mysqli->close();
?>