"""student URL Configuration

The `urlpatterns` list routes URLs to views. For more information please see:
    https://docs.djangoproject.com/en/3.2/topics/http/urls/
Examples:
Function views
    1. Add an import:  from my_app import views
    2. Add a URL to urlpatterns:  path('', views.home, name='home')
Class-based views
    1. Add an import:  from other_app.views import Home
    2. Add a URL to urlpatterns:  path('', Home.as_view(), name='home')
Including another URLconf
    1. Import the include() function: from django.urls import include, path
    2. Add a URL to urlpatterns:  path('blog/', include('blog.urls'))
"""
from django.urls import path
from myApp import views

urlpatterns = [
    path('', views.display, name='index'),
    path('remove/<int:pk>', views.deleteData, name='index'),
    path('create', views.inputData, name='create'),
    path('edit/<int:pk>', views.updateData, name='update'),
    path('search', views.searchData, name='search')
]
