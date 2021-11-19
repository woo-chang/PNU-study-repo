<?php
if (isset($_POST['mode'])) // 요런식으로 검사하면 값이 넘어왔는지 넘어오지 않았는지 확인 가능
$sql = "select ~~ " // 항상 해야하는 것, select로 DB 다 가져와서 그것을 출력
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
        var linkArr = new Array();
        var curPage = 0;

        function addLink()
        {
            // if (linkArr.findIndex(checkTitle) >= 0)
            // {
            //     window.alert("이미 있는 즐겨찾기 제목입니다. 다른 제목을 사용해주세요.");
            //     return;
            // }
            // linkArr.push({title: document.forms[0].linkTitle.value, src: document.forms[0].linkSrc.value});
            // linkArr.sort(compareTitle);

            // document.forms[0].reset();
            // makePages();
            // showLinks();
            // updateData();
        }
        // function checkTitle(link)
        // {
        //     return link.title == document.forms[0].linkTitle.value;
        // }
        // function compareTitle(link1, link2)
        // {
        //     if (link1.title > link2.title)
        //         return 1;
        //     else if (link1.title < link2.title)
        //         return -1;
        //     else
        //         return 0;
        // }       
        function deleteLink()
        {
            // var idx = linkArr.findIndex(checkTitle);
            // if (idx < 0)
            // {
            //     window.alert("일치하는 제목을 가진 즐겨찾기가 없습니다.");
            //     return;
            // }
            // linkArr.splice(idx, 1);
            // makePages();
            // showLinks();
            // updateData();
        }
        // function clearAll()
        // {
        //     linkArr = Array();
        //     document.getElementById("linkContainer").innerHTML = "";
        //     updateData();
        // }
        // function showLinks()
        // {
        //     var linkContainer = document.getElementById("linkContainer");
        //     linkContainer.innerHTML = "";

        //     var startIdx = curPage * 10;
        //     var endIdx = startIdx + 9;
        //     for(var idx = startIdx; idx <= endIdx && idx < linkArr.length; idx++)
        //     {
        //         var link = makeLink(linkArr[idx].title, linkArr[idx].src);
        //         linkContainer.innerHTML += link;
        //     }
        // }
        function makeLink(title, src)
        {
            //return "<a href='" + src + "' target=\"linkFrame\">" + title + "</a>";
        }
        function makePages()
        {
            if (linkArr.length > 10)
            {
                var pageNav = document.getElementById("pageNav");
                pageNav.style.visibility = "visible";
                pageNav.innerHTML = "";
                var nPage = Math.ceil(linkArr.length / 10);
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
            curPage = pageNum; // 페이지 정보도 hidden으로 넘겨서 확인
            makePages();
            //showLinks();
        }
        function updateData()
        {
            // localStorage.removeItem("links");
            // localStorage.setItem("links", JSON.stringify(linkArr));
        }
        function makeError()
        {
            var err = "<?php echo $err;?>"; // 에러가 출력
            // 주의할 점 : ""로 감싸고 해야 에러가 발생하지 않음
            if (err.length > 0)
            {
                window.alert(err);
            }
        } // 에러가 발생했으면 php 변수로 저장해두었다가 시작할 때 변수의 값을 출력하는 방법
        function start()
        {
            // var links = localStorage.getItem("links");
            // if (links != null)
            // {
            //     linkArr = JSON.parse(links);
            //     makePages();
            //     showLinks();
            // }
            makePages();
        }
    </script>
    </head>
<body onload="start()">
    <div class="maindiv">
        <form name="form1" method="GET" action="Q2.php">
            <label>즐겨찾기 제목: <input type="text" name="linkTitle" id="linkTitle"></label><br>
            <label>즐겨찾기 링크: <input type="text" name="linkSrc" id="linkSrc"></label><br>
            <input type="button" name="addBtn" id="addBtn" value="즐겨찾기 추가" onclick="addLink()">
            <input type="button" name="delBtn" id="delBtn" value="즐겨찾기 삭제" onclick="deleteLink()">
            <input type="reset" name="ClearBtn" id="clearBtn" value="모두 삭제" onclick="clearAll()">
            <!-- <input type="hidden">  hidden으로 버튼 정보를 넘겨주면 php에서 어디서 왔는지 확인 가능 -->
        </form>
        <br>
        <div id="linkContainer">
            <!-- 이전에는 DOM에 다 가져와서 출력했지만, 이제 php 바로 열어서 출력 가능 -->
            <?php
                echo "<a href='" . src . "' target=\"linkFrame\">" . title . "</a>";
                // 주의할 점 : 자바스크립트는 문자열 연결 '+', php는 문자열 연결 '.'
            ?>
        </div>
        <nav class="pagination" id="pageNav">
        </nav>
    </div>
    <iframe title="iframe for link" name="linkFrame"></iframe>
</body>
</html>