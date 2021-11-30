from django.db import models

# Create your models here.

class Students(models.Model):
    id = models.IntegerField(primary_key=True)
    firstname = models.CharField(max_length=25)
    secondname = models.CharField(max_length=25)
    age = models.IntegerField()
    major = models.CharField(max_length=50)
    address = models.CharField(max_length=50, blank=True, null=True)

    class Meta:
        managed = False
        db_table = 'Students'
