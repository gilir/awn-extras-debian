#! /usr/bin/python
# -*- coding: utf-8 -*-
#
# Copyright (c) 2008 sharkbaitbobby <sharkbaitbobby+awn@gmail.com>
#
# This library is free software; you can redistribute it and/or
# modify it under the terms of the GNU Lesser General Public
# License as published by the Free Software Foundation; either
# version 2 of the License, or (at your option) any later version.
#
# This library is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
# Lesser General Public License for more details.
#
# You should have received a copy of the GNU Lesser General Public
# License along with this library; if not, write to the
# Free Software Foundation, Inc., 59 Temple Place - Suite 330,
# Boston, MA 02111-1307, USA.
#
# File Browser Launcher
# Main Applet File

import sys
import os
import pygtk
pygtk.require('2.0')
import gtk
import subprocess
import pango
import urllib

import awn
import gconfwrapper as awnccwrapper

class App (awn.AppletSimple):
  def __init__(self, uid, orient, height):
    self.uid = uid
    
    #AWN Applet Configuration
    awn.AppletSimple.__init__(self, uid, orient, height)
    self.title = awn.awn_title_get_default()
    self.dialog = awn.AppletDialog(self)
    
    #Has to do with awncc
    self.client = awnccwrapper.AwnCCWrapper(self.uid)
    
    #Get the default icon theme
    self.theme = gtk.icon_theme_get_default()
    
    self.icon = self.set_awn_icon('file-browser-launcher', 'folder')
    
    #Make the dialog, will only be shown when approiate
    #VBox for everything to go in
    self.vbox = gtk.VBox()
    #Make all the things needed for a treeview for the homefolder, root dir, bookmarks, and mounted drives
    self.liststore = gtk.ListStore(gtk.gdk.Pixbuf,str)
    self.treeview = gtk.TreeView(self.liststore)
    self.treeview.set_hover_selection(True)
    self.renderer0 = gtk.CellRendererPixbuf()
    self.renderer1 = gtk.CellRendererText()
    self.treeview.set_headers_visible(False)
    self.column0 = gtk.TreeViewColumn('0')
    self.column0.pack_start(self.renderer0,True)
    self.column0.add_attribute(self.renderer0,'pixbuf',0)
    self.column1 = gtk.TreeViewColumn('1')
    self.column1.pack_start(self.renderer1,True)
    self.column1.add_attribute(self.renderer1,'markup',1)
    self.treeview.append_column(self.column0)
    self.treeview.append_column(self.column1)
    self.treeview.connect('button-press-event',self.treeview_clicked)
    self.vbox.pack_start(self.treeview)
    
    #Entry widget for displaying the path to open
    self.entry = gtk.Entry()
    self.entry.set_text(os.path.expanduser('~'))
    self.entry.connect('key-release-event',self.detect_enter)
    #Open button to run the file browser
    self.enter = gtk.Button(stock=gtk.STOCK_OPEN)
    self.enter.connect('clicked',self.launch_fb)
    #HBox to put the two together
    self.hbox = gtk.HBox()
    self.hbox.pack_start(self.entry)
    self.hbox.pack_start(self.enter,False)
    #And add the HBox to the vbox and add the vbox to the dialog
    self.vbox.pack_start(self.hbox)
    self.dialog.add(self.vbox)
    
    #AWN applet signals
    self.connect('button-press-event', self.button_press)
    self.connect('enter-notify-event', lambda a,b: self.title.show(self,'File Browser Launcher'))
    self.connect('leave-notify-event', lambda a,b: self.title.hide(self))
    self.dialog.connect('focus-out-event', lambda a,b: self.dialog.hide())
  
  #Function to show the home folder, mounted drives/partitions, and bookmarks according to awncc
  #This also refreshes in case a CD was inserted, MP3 player unplugged, bookmark added, etc.
  def add_places(self):
    #This function adds items to the liststore. The TreeView was already made in __init__()
    
    #Empty the liststore if it isn't
    self.liststore.clear()
    self.places_paths = []
    
    #Get the needed awncc values
    self.show_home = self.client.get_int('places_home',2)
    self.show_local = self.client.get_int('places_local',2)
    self.show_network = self.client.get_int('places_network',2)
    self.show_bookmarks = self.client.get_int('places_bookmarks',2)
    
    #Check to see if we should check /etc/fstab and $mount
    self.do_mounted = False
    if 2 in [self.show_local, self.show_network]:
      self.do_mounted = True
    
    #Now make the actual mounted items. First: Home Folder
    if self.show_home==2:
      self.icon_home = self.theme.load_icon('user-home',24,24)
      try:
        self.liststore.append([self.icon_home,'Home Folder'])
      except:
        self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
          .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'Home Folder'])
      self.places_paths.append(os.path.expanduser('~'))
    
    #Get list of mounted drives from $mount and /etc/fstab
    if self.do_mounted:
      self.mount2 = os.popen('mount')
      self.mount = self.mount2.readlines()
      self.mount2.close()
      self.fstab2 = open('/etc/fstab','r')
      self.fstab = self.fstab2.read().split('\n')
      self.fstab2.close()
      self.paths_fstab = []
      self.network_paths = []
      self.network_corr_hnames = []
      self.cd_paths = []
      self.dvd_paths = []
    
    #Get list of bookmarks
    self.bmarks2 = open(os.path.expanduser('~/.gtk-bookmarks'))
    self.bmarks = self.bmarks2.readlines()
    self.bmarks2.close()
    
    #Set list of paths, regardless of location
    self.paths = []
    self.paths_hnames = []
    
    #Get whether the trash is empty or not - but first find out if the Trash is in
    #~/.Trash or ~/.local/share/Trash
    try:
      #Get trash dir
      if os.path.isdir(os.path.expanduser('~/.local/share/Trash/files')):
        self.trash_path = os.path.expanduser('~/.local/share/Trash/files')
      else:
        self.trash_path = os.path.expanduser('~/.Trash')
      
      #Get number of items in trash
      if len(os.listdir(self.trash_path)) > 0:
        self.trash_full = True
      else:
        self.trash_full = False
    
    except:
      #Maybe the trash is in a different location? Just put false
      self.trash_full = False
    
    #Get the mounted drives/partitions that are suitable to list (from fstab)
    if self.do_mounted:
      for x in self.fstab:
        try:
          if x.replace(' ','').replace('\t','')!='' and x[0]!="#":
            y = x.split(' ')
            for z in y[1:]:
              if z!='':
                if z[0]=='/':
                  if z!='/proc':
                    self.paths_fstab.append(z)
            z = x.replace('  ',' ').split(' ')
            z2 = []
            for z3 in z:
              z2.extend(z3.split('\t'))
            if z2[2] == 'smbfs':
              #print "SMBFS:", z2
              self.network_paths.append('smb:'+z2[0])
              self.network_corr_hnames.append(z2[0].split(':')[-1].split('/')[-1]+\
                ' on '+z2[0].split('/')[2])
            elif z2[2] in ['cifs','nfs','ftpfs','sshfs']:
              self.network_paths.append(z2[1])
              self.network_corr_hnames.append(z2[0].split(':')[-1].split('/')[-1]+\
                ' on '+z2[0].split('/')[2])
        except:
          #Maybe a syntax error or something in this line of fstab?
          #Just ignore it (better than not working at all (thanks Kinap/Felix)
          pass
      
      #Get the mounted drives/partitions that are suitable to list (from mount)
      for x in self.mount:
        y = x.split(' ')
        if y[0].find('/')!=-1:
          if y[0].split('/')[1]=='dev':
            self.paths.append(x.split(' on ')[1].split(' type ')[0])
            if x[-1]==']':
              self.paths_hnames.append(x.split('[')[-1][:-1])
            else:
              self.paths_hnames.append(x.split(' on ')[1].split(' type ')[0]\
                .split('/')[-1])
            if x.split(' type ')[1].split(' ')[0]=='iso9660':
              self.cd_paths.append(x.split(' on ')[1].split(' type ')[0])
            elif x.split(' type ')[1].split(' ')[0]=='udf':
              self.dvd_paths.append(x.split(' on ')[1].split(' type ')[0])
    
    #Go through the list and get the right icon and name for specific ones
    #ie/eg: / -> harddisk icon and "Filesystem"
    #/media/Lexar -> usb-disk icon and "Lexar"
    #TODO: Clean this up (oh it's so ugly)
    if self.show_local==2:
      for x in self.paths:
        if x=='/':
          try:
            self.liststore.append([self.theme.load_icon('drive-harddisk',24,24),'Filesystem'])
          except:
            self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
              .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'Filesystem'])
          self.places_paths.append(x)
        elif x.split('/')[1]=='media':
          if x.split('/')[2] in ['cdrom0','cdrom1','cdrom2','cdrom3','cdrom4','cdrom5']:
            #Find out if it's a CD or DVD
            if x in self.dvd_paths:
              try:
                self.liststore.append([self.theme.load_icon('media-optical',24,24),'DVD Drive'])
              except:
                self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                  .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'DVD Drive'])
              self.places_paths.append(x)
            else:
              try:
                self.liststore.append([self.theme.load_icon('media-optical',24,24),'CD Drive'])
              except:
                self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                  .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'CD Drive'])
              self.places_paths.append(x)
          elif x not in self.paths_fstab: #Means it's USB or firewire
            try:
              self.liststore.append([self.theme.load_icon('gnome-dev-harddisk-usb',24,24),\
                self.paths_hnames[self.paths.index(x)]])
            except:
              self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),\
                  self.paths_hnames[self.paths.index(x)]])
            self.places_paths.append(x)
          else: #Regular mounted drive (ie/eg windows partition)
            try:
              self.liststore.append([self.theme.load_icon('drive-harddisk',24,24),\
                self.paths_hnames[self.paths.index(x)]])
            except:
              self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),self.paths_hnames[self.paths.index(x)]])
            self.places_paths.append(x)
        else: #Maybe /home, /boot, /usr, etc.
          try:
            self.liststore.append([\
              self.theme.load_icon('drive-harddisk',24,24),self.paths_hnames[self.paths.index(x)]])
          except:
            self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
              .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),self.paths_hnames[self.paths.index(x)]])
          self.places_paths.append(x)
    
    #Go through the list of network drives/etc. from /etc/fstab
    if self.show_network==2:
      #print self.network_paths
      #GVFS stuff
      if os.path.isdir(os.path.expanduser('~/.gvfs')):
        for x in os.listdir(os.path.expanduser('~/.gvfs')):
          try:
            self.liststore.append([self.theme.load_icon('network-folder',24,24),\
              x])
          except:
            self.liststore.append([gtk.gdk.pixbuf_new_from_file(\
              self.default_icon_path).scale_simple(\
              24,24,gtk.gdk.INTERP_BILINEAR),x])
          self.places_paths.append(os.path.expanduser('~/.gvfs')+'/'+x)
      #Non-GVFS stuff
      y = 0
      for x in self.network_paths:
        try:
          self.liststore.append([self.theme.load_icon('network-folder',24,24),\
            self.network_corr_hnames[y]])
        except:
          self.liststore.append([gtk.gdk.pixbuf_new_from_file(\
            self.default_icon_path).scale_simple(\
            24,24,gtk.gdk.INTERP_BILINEAR),self.network_corr_hnames[y]])
        self.places_paths.append(x)
        y+=1
        
      
    
    #Go through the list of bookmarks and add them to the list IF it's not in the mount list
    if self.show_bookmarks==2:
      for x in self.bmarks:
        x = x.replace('file://','').replace('\n','')
        x = urllib.unquote(x)
        if x not in self.paths and x!=os.path.expanduser('~'):
          if x[0]=='/': #Normal filesystem bookmark, not computer:///,burn:///,network:///,etc.
            if os.path.isdir(self.parse_bookmark(x,'path')):
              try:
                self.liststore.append([self.theme.load_icon('folder',24,24),self.parse_bookmark(x,'name')])
              except:
                self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                  .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),self.parse_bookmark(x,'name')])
              self.places_paths.append(self.parse_bookmark(x,'path'))
          else:
            y = x.split(':')[0]
            if y=='computer':
              try:
                self.liststore.append([self.theme.load_icon('computer',24,24),'Computer'])
              except:
                self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                  .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'Computer'])
              self.places_paths.append('%s:///' % y)
            elif y in ['network','smb','nfs','ftp','ssh']:
              try:
                self.liststore.append([self.theme.load_icon('network-server',24,24),'Network'])
              except:
                self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                  .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'Network'])
              self.places_paths.append('%s:///' % y)
            elif y=='trash':
              if self.trash_full==True:
                try:
                  self.liststore.append([self.theme.load_icon('user-trash-full',24,24),'Trash'])
                except:
                  self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                    .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'Trash'])
                self.places_paths.append('%s:///' % y)
              else:
                try:
                  self.liststore.append([self.theme.load_icon('user-trash',24,24),'Trash'])
                except:
                  self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                    .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'Trash'])
                self.places_paths.append('%s:///' % y)
            elif y=='x-nautilus-search':
              try:
                self.liststore.append([self.theme.load_icon('search',24,24),'Search'])
              except:
                self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                  .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'Search'])
              self.places_paths.append('%s:///' % y)
            elif y=='burn':
              try:
                self.liststore.append([self.theme.load_icon('drive-optical',24,24),'CD/DVD Burner'])
              except:
                self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                  .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'CD/DVD Burner'])
              self.places_paths.append('%s:///' % y)
            elif y=='fonts':
              try:
                self.liststore.append([self.theme.load_icon('font',24,24),'Fonts'])
              except:
                self.liststore.append([gtk.gdk.pixbuf_new_from_file(self.default_icon_path)\
                  .scale_simple(24,24,gtk.gdk.INTERP_BILINEAR),'Fonts'])
              self.places_paths.append('%s:///' % y)
  
  #Parses the text of a line of ~/.gtk-bookmarks after the file:/// and gets the real filepath or the name of it
  def parse_bookmark(self,string,pathorname):
    if pathorname=='path':
      return string.split(' ')[0].replace('%20',' ')
    elif pathorname=='name':
      try:
        if ' '.join(string.split(' ')[1:]).replace(' ','')=='':
          return string.split('/')[-1].replace('%20',' ')
        else:
          return ' '.join(string.split(' ')[1:])
      except:
        return string.split('/')[-1].replace('%20',' ')
  
  #Function to do what should be done according to awncc when the treeview is clicked
  def treeview_clicked(self,widget,event):
    self.open_clicked = self.client.get_int('places_open',2)
    self.selection = self.treeview.get_selection()
    if self.open_clicked==2:
      self.dialog.hide()
      self.launch_fb(None,self.places_paths[self.liststore[self.selection.get_selected()[1]].path[0]])
    else:
      self.entry.set_text(self.places_paths[self.liststore[self.selection.get_selected()[1]].path[0]])
      self.entry.grab_focus()
  
  #Applet show/hide methods - copied from MiMenu (and edited)
  #When a button is pressed
  def button_press(self, widget, event):
    if self.dialog.flags() & gtk.VISIBLE:
      self.dialog.hide()
      self.title.hide(self)
    else:
      if event.button==1 or event.button==2:
        self.dialog_config(event.button)
      elif event.button==3:
        self.show_menu(event)
      self.title.hide(self)
  
  #dialog_config: 
  def dialog_config(self,button):
    if button!=1 and button!=2:
      return False
    self.curr_button = button
    
    #Get whether to focus the entry when displaying the dialog or not
    self.awncc_focus = self.client.get_int('focus_entry',2)
    
    if button==1: #Left mouse button
    #Get the value for the left mouse button to automatically open. Create and default to 1 the entry if it doesn't exist
    #Also get the default directory or default to ~
      self.awncc_lmb = self.client.get_int('lmb',1)
      self.awncc_lmb_path = self.client.get_string('lmb_path',\
      os.path.expanduser('~'))
      self.awncc_lmb_path = self.convert_home(self.awncc_lmb_path)
    
    elif button==2: #Middle mouse button
    #Get the value for the middle mouse button to automatically open. Create and default to 2 the entry if it doesn't exist
    #Also get the default directory or default to ~
      self.awncc_mmb = self.client.get_int('mmb',2)
      self.awncc_mmb_path = self.client.get_string('mmb_path',\
      os.path.expanduser('~'))
      self.awncc_mmb_path = self.convert_home(self.awncc_mmb_path)
    
    #Now get the chosen program for file browsing from awncc
    self.awncc_fb = self.client.get_string('fb','xdg-open')
    
    #Left mouse button - either popup with correct path or launch correct path OR do nothing
    if button==1:
      if self.awncc_lmb==1:
        self.entry.set_text(self.awncc_lmb_path)
        self.add_places()
        if self.awncc_focus==2:
          self.entry.grab_focus()
          self.entry.set_position(-1)
        self.dialog.show_all()
      elif self.awncc_lmb==2:
        self.launch_fb(None,self.awncc_lmb_path)
    
    #Right mouse button - either popup with correct path or launch correct path OR do nothing
    if button==2:
      if self.awncc_mmb==1:
        self.entry.set_text(self.awncc_mmb_path)
        self.add_places()
        if self.awncc_focus==2:
          self.entry.grab_focus()
          self.entry.set_position(-1)
        self.dialog.show_all()
      elif self.awncc_mmb==2:
        self.launch_fb(None,self.awncc_mmb_path)
  
  #~/etc -> [actual home path]/etc
  def convert_home(self,tidle):
    return os.path.expanduser(tidle)
  
  #If the user hits the enter key on the main part OR the number pad
  def detect_enter(self,a,event):
    if event.keyval==65293 or event.keyval==65421:
      self.enter.clicked()
  
  #Launces file browser to open "path". If "path" is None: use value from the entry widget
  def launch_fb(self,widget,path=None):
    self.dialog.hide()
    if path==None:
      path = self.entry.get_text()
    
    #Get the file browser app, or set to xdg-open if it's not set
    self.awncc_fb = self.client.get_string('fb','xdg-open')
    
    #In case there is nothing but whitespace (or at all) in the entry widget
    if path.replace(' ','')=='':
      path = os.path.expanduser('~')
    
    #Launch file browser at path
    #print "Running:", self.awncc_fb+' '+path.replace(' ','\ ')
    subprocess.Popen(self.awncc_fb+' '+path.replace(' ','\ '),shell=True)
  
  #Right click menu - Preferences or About
  def show_menu(self,event):
    
    #Hide the dialog if it's shown
    self.dialog.hide()
    
    #Create the items for Preferences and About
    self.prefs = gtk.ImageMenuItem(gtk.STOCK_PREFERENCES)
    self.about = gtk.ImageMenuItem(gtk.STOCK_ABOUT)
    
    #Connect the two items to functions when clicked
    self.prefs.connect("activate",self.open_prefs)
    self.about.connect("activate",self.open_about)
    
    #Now create the menu to put the items in and show it
    self.menu = self.create_default_menu()
    self.menu.append(self.prefs)
    self.menu.append(self.about)
    self.menu.show_all()
    self.menu.popup(None, None, None, event.button, event.time)
  
  #Show the preferences window
  def open_prefs(self,widget):
    #Import the prefs file from the same directory
    import prefs
    
    #Show the prefs window - see prefs.py
    prefs.Prefs(self)
    gtk.main()
  
  #Show the about window
  def open_about(self,widget):
    #Import the about file from the same directory
    import about
    
    #Show the about window - see about.py
    about.About()
  
    
if __name__ == '__main__':
  awn.init(sys.argv[1:])
  applet = App(awn.uid, awn.orient,awn.height)
  awn.init_applet(applet)
  applet.show_all()
  gtk.main()
