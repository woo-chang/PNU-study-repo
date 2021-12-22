from django.shortcuts import render
from django.shortcuts import redirect
from django.db import connection
import csv

# Create your views here.

def display(request):
    fetchStudents = []
    sendStudents = []
    fetchProfessors = []
    sendProfessors = []
    fetchCounties = []
    sendCounties = []
    fetchCOVID = []
    sendCOVID = []

    sendQuery1 = []
    sendQuery2 = []
    sendQuery3 = []
    sendQuery4 = []
    sendQuery5 = []

    try:
        sql_object = connection.cursor()
        my_query = """
        Select * from Students
        """
        result = sql_object.execute(my_query)
        fetchStudents = sql_object.fetchall()  # 쿼리 실행 결과로 반환된 전체 데이터를 데이터베이스 서버로부터 가져옴
        my_query = """
        Select * from Professors
        """
        result = sql_object.execute(my_query)
        fetchProfessors = sql_object.fetchall()  # 쿼리 실행 결과로 반환된 전체 데이터를 데이터베이스 서버로부터 가져옴
        my_query = """
        Select * from Counties
        """
        result = sql_object.execute(my_query)
        fetchCounties = sql_object.fetchall()  # 쿼리 실행 결과로 반환된 전체 데이터를 데이터베이스 서버로부터 가져옴
        my_query = """
        Select * from COVID
        """
        result = sql_object.execute(my_query)
        fetchCOVID = sql_object.fetchall()  # 쿼리 실행 결과로 반환된 전체 데이터를 데이터베이스 서버로부터 가져옴

    except:
        connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
        print("Select DATA 실패")

    finally:
        print("Select DATA 종료")
        connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
        connection.close()  # 데이터베이스와의 연결을 종료

    if exist_data(fetchStudents, fetchProfessors, fetchCounties, fetchCOVID):
        sendQuery1 = my_query1()
        sendQuery2 = my_query2()
        sendQuery3 = my_query3()
        sendQuery4 = my_query4()
        sendQuery5 = my_query5()

    for student in fetchStudents:
        sendStudents.append({'studentID': student[0], 'name': student[1], 'score': student[2], 'county': student[3]})

    for professor in fetchProfessors:
        sendProfessors.append({'facultyID': professor[0], 'name': professor[1], 'age': professor[2], 'county': professor[3]})

    for county in fetchCounties:
        sendCounties.append({'countyName': county[0], 'population': county[1], 'city': county[2]})

    for covid in fetchCOVID:
        sendCOVID.append({'patientID': covid[0], 'city': covid[1]})

    return render(request, 'myApp/index.html', {"Students": sendStudents, "Professors": sendProfessors, "Counties": sendCounties, "COVID": sendCOVID, "Query1": sendQuery1, "Query2": sendQuery2, "Query3": sendQuery3, "Query4": sendQuery4, "Query5": sendQuery5})


def csvStudents(request):
    with open('./myApp/templates/myApp/students.csv', mode='r') as students_lists:
        student_reader = csv.reader(students_lists)

        try:
            sql_object = connection.cursor()

            for student in student_reader:
                student_id = student[0]
                name = student[1]
                score = student[2]
                county = student[3]
                my_query = f"""
                INSERT INTO Students(`studentID`, `name`, `score`, `county`)
                VALUES ('{student_id}', '{name}', '{score}', '{county}');
                """
                result = sql_object.execute(my_query)

        except:
            connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
            print("Insert Students 실패")

        finally:
            print("Insert Student 종료")
            connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
            connection.close()  # 데이터베이스와의 연결을 종료

    return redirect('index')


def csvProfessors(request):
    with open('./myApp/templates/myApp/professors.csv', mode='r') as professors_list:
        professor_reader = csv.reader(professors_list)

        try:
            sql_object = connection.cursor()

            for professor in professor_reader:
                faculty_id = professor[0]
                name = professor[1]
                age = professor[2]
                county = professor[3]
                my_query = f"""
                INSERT INTO Professors(`facultyID`, `name`, `age`, `county`)
                VALUES ('{faculty_id}', '{name}', '{age}', '{county}');
                """
                result = sql_object.execute(my_query)

        except:
            connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
            print("Insert Professors 실패")

        finally:
            print("Insert Professors 종료")
            connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
            connection.close()  # 데이터베이스와의 연결을 종료

    return redirect('index')


def csvCounties(request):
    with open('./myApp/templates/myApp/counties.csv', mode='r') as counties_list:
        county_reader = csv.reader(counties_list)

        try:
            sql_object = connection.cursor()

            for county in county_reader:
                countyName = county[0]
                population = county[1]
                city = county[2]
                my_query = f"""
                INSERT INTO Counties(`countyName`, `population`, `city`)
                VALUES ('{countyName}', '{population}', '{city}');
                """
                result = sql_object.execute(my_query)

        except:
            connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
            print("Insert Counties 실패")

        finally:
            print("Insert Counties 종료")
            connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
            connection.close()  # 데이터베이스와의 연결을 종료

    return redirect('index')


def csvCovid(request):
    with open('./myApp/templates/myApp/covid.csv', mode='r') as covid_list:
        covid_reader = csv.reader(covid_list)

        try:
            sql_object = connection.cursor()

            for covid in covid_reader:
                patient_id = covid[0]
                city = covid[1]
                my_query = f"""
                INSERT INTO COVID(`patientID`, `city`)
                VALUES ('{patient_id}', '{city}');
                """
                result = sql_object.execute(my_query)

        except:
            connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
            print("Insert COVID 실패")

        finally:
            print("Insert COVID 종료")
            connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
            connection.close()  # 데이터베이스와의 연결을 종료

    return redirect('index')


def exist_data(data1, data2, data3, data4):
    if data1 and data2 and data3 and data4:
        return True
    else:
        return False


def my_query1():
    my_return = []

    try:
        sql_object = connection.cursor()

        my_query = f"""
        select countyName, AVG(score)
        from students, counties
        where students.county = counties.countyName
        group by countyName
        """
        result = sql_object.execute(my_query)
        fetchResult = sql_object.fetchall()

        for fetch in fetchResult:
            my_return.append({"countyName": fetch[0], "averageScore": round(fetch[1], 4)})

    except:
        connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
        print("Query1 실패")

    finally:
        print("Query1 종료")
        connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
        connection.close()  # 데이터베이스와의 연결을 종료

    return my_return


def my_query2():
    my_return = []

    try:
        sql_object = connection.cursor()

        my_query = f"""
        select city, AVG(score)
        from students, counties
        where students.county = counties.countyName
        group by city
        """
        result = sql_object.execute(my_query)
        fetchResult = sql_object.fetchall()

        for fetch in fetchResult:
            my_return.append({"cityName": fetch[0], "averageScore": round(fetch[1], 4)})

    except:
        connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
        print("Query2 실패")

    finally:
        print("Query2 종료")
        connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
        connection.close()  # 데이터베이스와의 연결을 종료

    return my_return


def my_query3():
    my_return = []

    try:
        sql_object = connection.cursor()

        my_query = f"""
            select p1.name, s1.name
            from students s1
            inner join(
                select countyName, MAX(score) as score
                from students, counties
                where students.county = counties.countyName
                group by countyName
            ) s2 on s1.county = s2.countyName and s1.score = s2.score,
            professors p1
            inner join(
                select countyName, Max(age) as age
                from professors, counties
                where professors.county = counties.countyName
                group by countyName
            ) p2 on p1.county = p2.countyName and p1.age = p2.age
            where s1.county = p1.county
        """

        result = sql_object.execute(my_query)
        fetchResult = sql_object.fetchall()

        for fetch in fetchResult:
            my_return.append({"professorName": fetch[0], "studentName": fetch[1]})

    except:
        connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
        print("Query3 실패")

    finally:
        print("Query3 종료")
        connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
        connection.close()  # 데이터베이스와의 연결을 종료

    return my_return


def my_query4():
    my_return = []

    try:
        sql_object = connection.cursor()

        my_query = f"""
            select p.name, s.name
            from students s
            inner join counties c1 on s.county = c1.countyName
            inner join (
                select city, MAX(score) as score
                from students, counties
                where students.county = counties.countyName
                group by city
            ) cs on s.score = cs.score and c1.city = cs.city,
            professors p
            inner join counties c2 on p.county = c2.countyName
            inner join (
                select city, MAX(age) as age
                from professors, counties
                where professors.county = counties.countyName
                group by city
            ) ca on p.age = ca.age and c2.city = ca.city
            where cs.city = ca.city
        """

        result = sql_object.execute(my_query)
        fetchResult = sql_object.fetchall()

        for fetch in fetchResult:
            my_return.append({"professorName": fetch[0], "studentName": fetch[1]})

    except:
        connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
        print("Query4 실패")

    finally:
        print("Query4 종료")
        connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
        connection.close()  # 데이터베이스와의 연결을 종료

    return my_return


def my_query5():
    my_return = []

    # query 5 start
    try:
        sql_object = connection.cursor()

        my_query = f"""
            select s.name, c2.city, topCity.rate as rate
            from students s
            inner join counties c2 on s.county = c2.countyName,
            (
                select c.city as city, (cityCovid.patientCount/Sum(c.population)) as rate
                from counties c, (
                    select cityList.city as city, COUNT(patientID) as patientCount
                    from covid, (
                        select city
                        from counties
                        group by city
                    ) cityList
                    where covid.city = cityList.city
                    group by cityList.city
                ) cityCovid
                where c.city = cityCovid.city
                group by c.city
                order by rate desc
                limit 3
            ) topCity
            where c2.city = topCity.city
            order by rate desc
        """

        result = sql_object.execute(my_query)
        fetchResult = sql_object.fetchall()

        for fetch in fetchResult:
            my_return.append({"studentName": fetch[0], "cityName": fetch[1]})

    except:
        connection.rollback()  # 쿼리문 도중 잘못된 실행이 발생할 경우 실행 전으로 되돌려 놓음
        print("Query5 실패")

    finally:
        print("Query5 종료")
        connection.commit()  # 데이터베이스 변경사항이 존재하면 이를 확정, 갱신
        connection.close()  # 데이터베이스와의 연결을 종료

    return my_return