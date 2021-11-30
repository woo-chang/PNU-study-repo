from django.shortcuts import render
from .models import Categories

def display(request):
    categories = Categories.objects.all()
    return render(request, 'myApp/index.html',{"categories": categories})
