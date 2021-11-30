from django.shortcuts import render
from django.shortcuts import redirect
from .models import Students
from .forms import StudentsForm
from django.contrib import messages

# Create your views here.

def display(request):
    students = Students.objects.all()
    return render(request, 'myApp/index.html', {"students": students})

def inputData(request):
    if request.method == "POST":
        form = StudentsForm(request.POST)
        if form.is_valid():
            form.save()
            return redirect('/')
        messages.warning(request, '올바르게 입력해주세요.')
    else:
        form = StudentsForm()
    return render(request, "myApp/input.html", {'form': form})

def deleteData(request, pk):
    student = Students.objects.get(pk=pk)
    if request.method == "GET":
        student.delete()
        return redirect('/')
    students = Students.objects.all()
    return render(request, 'myApp/index.html', {"students": students})

def updateData(request, pk):
    form = StudentsForm()
    if request.method == "POST":
        student = Students.objects.get(pk=pk)
        student.firstname = request.POST.get('firstname')
        student.secondname = request.POST.get('secondname')
        student.age = request.POST.get('age')
        student.major = request.POST.get('major')
        student.address = request.POST.get('address')
        student.save()
        return redirect('/')
    else:
        form = StudentsForm()
    return render(request, "myApp/update.html", {'form': form, 'id': pk})

def searchData(request):
    students = None
    if 'q' in request.GET:
        q = request.GET['q']
        if q:
            students = Students.objects.all()
            students = students.filter(pk=q)
            if not students:
                messages.warning(request, 'No records found')
        else:
            messages.warning(request, 'No records found')
    return render(request, 'myApp/search.html', {"students": students})