APPLET_NAME = hardware-sensors
APPLET_MAIN_FILE = hardware-sensors.py
include $(top_srcdir)/Makefile.python-applet
include $(top_srcdir)/Makefile.schemas

dist_applet_DATA =		\
	sensoricon.py		\
	hardware-sensors.ui	\
	$(NULL)

interfacesdir = $(applet_datadir)/interfaces
dist_interfaces_DATA =				\
	interfaces/__init__.py			\
	interfaces/acpisensors.py		\
	interfaces/fileinterface.py		\
	interfaces/hddtempsensors.py	\
	interfaces/lmsensors.py			\
	interfaces/nvclocksensors.py	\
	interfaces/nvidiasensors.py		\
	interfaces/omnibooksensors.py	\
	interfaces/sensorinterface.py	\
	$(NULL)

sensorvaluesdir = $(applet_datadir)/sensorvalues
dist_sensorvalues_DATA =		\
	sensorvalues/__init__.py	\
	sensorvalues/rpmvalue.py	\
	sensorvalues/sensorvalue.py	\
	sensorvalues/tempvalue.py	\
	sensorvalues/units.py		\
	sensorvalues/voltvalue.py	\
	$(NULL)

imagesdir = $(applet_datadir)/images
dist_images_DATA =			\
	images/no_sensors.svg	\
	images/thermometer.png	\
	images/thermometer.svg	\
	$(NULL)

theme_darkdir = $(applet_datadir)/themes/dark
dist_theme_dark_DATA =		\
	themes/dark/double.svg	\
	themes/dark/single.svg	\
	$(NULL)

theme_transparentdir = $(applet_datadir)/themes/transparent
dist_theme_transparent_DATA =		\
	themes/transparent/double.svg	\
	themes/transparent/single.svg	\
	$(NULL)

theme_transparent_with_borderdir = $(applet_datadir)/themes/transparent_with_border
dist_theme_transparent_with_border_DATA =		\
	themes/transparent_with_border/double.svg	\
	themes/transparent_with_border/single.svg	\
	$(NULL)
	