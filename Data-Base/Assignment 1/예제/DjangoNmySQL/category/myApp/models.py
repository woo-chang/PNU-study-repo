from django.db import models

# Create your models here.
class Categories(models.Model):
    categoryid = models.IntegerField(db_column='categoryID', primary_key=True)  # Field name made lowercase.
    categoryname = models.CharField(db_column='categoryName', max_length=50, blank=True, null=True)  # Field name made lowercase.
    categorydescription = models.CharField(db_column='categoryDescription', max_length=100, blank=True, null=True)  # Field name made lowercase.

    class Meta:
        managed = False
        db_table = 'categories'
