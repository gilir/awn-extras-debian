#!/usr/bin/env python
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
#To-Do List Applet
#Applet file

#Gtk, etc. stuff
import gobject
import pygtk
pygtk.require('2.0')
import gtk
import cairo

#This applet stuff
import settings
import icon

#Awn stuff
import sys
import awn
from awn.extras import detach, surface_to_pixbuf

class App(awn.AppletSimple):
  last_num_items = -1
  surface = None
  last_height = -1
  progress_buttons = []
  
  def __init__(self, uid, orient, height):
    self.uid = uid
    self.height = height
    
    #Values that will be referenced later
    self.displayed = False
    self.detached = False
    
    #AWN Applet Configuration
    awn.AppletSimple.__init__(self,uid,orient,height)
    self.title = awn.awn_title_get_default()
    self.dialog = awn.AppletDialog(self)
    
    #Give the dialog an AccelGroup (is this all that necessary)
    self.accel = gtk.AccelGroup()
    self.dialog.add_accel_group(self.accel)
    
    #Set up Settings
    self.settings = settings.Settings('to-do', uid)
    self.settings.register({ \
      'color':(str, 'skyblue'), \
      'title':(str, ("To-Do List")), \
      'items':([str], []), \
      'colors':([int], []), \
      'icon-type':(str, 'items'), \
      'details':([str], []), \
      'color_low':(str, '#009900'), \
      'color_med':(str, '#c0c000'), \
      'color_high':(str, '#aa0000'), \
      'progress':([int], []), \
      'priority':([int], []), \
      'expanded':([int], []), \
      'category':([int], []), \
      'custom_width':(int, 125), \
      'icon-opacity':(int, 100), \
      'color_low_text':(str, '#000000'), \
      'color_med_text':(str, '#000000'), \
      'confirm-items':(bool, True), \
      'category_name':([str], []), \
      'color_high_text':(str, '#dddddd'), \
      'use_custom_width':(bool, False), \
      'confirm-categories':(bool, True)})
    
    #Icon Type
    if self.settings['icon-type'] not in ['progress','progress-items','items']:
      self.settings['icon-type'] = 'items'
    
    #Icon opacity
    if self.settings['icon-opacity'] < 10 or \
      self.settings['icon-opacity'] > 100:
      self.settings['icon-opacity'] = 90
    
    #Get the Icon Theme - used for the "X" to remove an item
    #and the > arrow to edit details of an item
    self.icon_theme = gtk.icon_theme_get_default()
    
    #Set up the drawn icon - colors and stuff
    
    #Get the icon color
    #One of the Tango Desktop Project Color Palatte colors
    if self.settings['color'] in ['butter','chameleon','orange','skyblue',\
      'plum','chocolate','scarletred','aluminium1','aluminium2']:
      self.color = icon.colors[self.settings['color']]
    
    #Custom colors
    elif self.settings['color'] == 'custom':
      self.update_custom_colors()
    
    #Gtk Theme colors
    elif self.settings['color'] == 'gtk':
      self.update_icon_theme()
    
    #No or invalid color set
    #Default to "Sky Blue" (My Favorite ;)
    else:
      self.settings['color'] = 'skyblue'
      self.color = icon.colors['skyblue']
    
    #Set up detach (settings, etc. is done a little later)
    self.detach = detach.Detach()
    
    #Setup the icon
    self.update_icon()
    
    #Set some settings
    self.detach['applet-right-click'] = 'signal'
    self.detach['right-click'] = 'signal'
    
    #Connect to some signals
    self.detach.connect('hide-awn-icon',self.hide_icon)
    self.detach.connect('attach',self.was_attached)
    self.detach.connect('detach',self.was_detached)
    self.detach.connect('applet-toggle-dialog',self.toggle_dialog)
    self.detach.connect('show-dialog',self.show_dialog)
    self.detach.connect('hide-dialog',self.hide_dialog)
    self.detach.connect('applet-right-click',self.show_menu)
    self.detach.connect('right-click',self.show_menu)
    self.detach.connect(['scroll-up','scroll-right'],self.opacity,True)
    self.detach.connect(['scroll-down','scroll-left'],self.opacity,False)
    
    #Prepare the applet for dragging from Awn
    self.detach.prepare_awn_drag_drop(self)
    
    #Connect to events
    self.connect('enter-notify-event', self.show_title)
    self.connect('leave-notify-event',\
      lambda *a: self.title.hide(self))
    self.connect('height-changed', self.height_changed)
    self.dialog.connect('focus-out-event',self.hide_dialog)
    self.settings.connect('items',self.update_icon)
    self.settings.connect('progress',self.update_icon)
    self.settings.connect('color', self.update_icon)
    self.settings.connect('colors', self.update_icon)
    self.settings.connect('icon-type', self.update_icon)
  
  #Remove anything shown in the dialog - does not hide the dialog
  def clear_dialog(self,*args):
    for pb in self.progress_buttons:
      pb.disconn()
      pb.destroy()
    
    while True:
      try:
        del self.progress_buttons[0]
      except:
        break
    
    try:
      self.dialog_widget.destroy()
    except:
      pass
  
  #Add a widget to the dialog - detached or not
  def add_to_dialog(self,widget):
    self.dialog_widget = widget
    if self.detached == True:
      self.detach.add(self.dialog_widget)
    else:
      self.dialog.add(self.dialog_widget)
  
  #The height of Awn has changed
  def height_changed(self, applet, new_height):
    self.height = new_height
    self.last_num_items = -1
    self.update_icon()
  
  #Display a right-click context menu
  def show_menu(self,event):
    #Hide the dialog if it's shown
    self.hide_dialog()
    
    #Create the items for Preferences, Detach, and About
    prefs_menu = gtk.ImageMenuItem(gtk.STOCK_PREFERENCES)
    detach_menu = self.detach.menu_item(self.do_detach,self.do_attach)
    about_menu = gtk.ImageMenuItem(gtk.STOCK_ABOUT)
    
    #Connect the two items to functions when selected
    prefs_menu.connect('activate',self.show_prefs)
    about_menu.connect('activate',self.show_about)
    
    #Now create the menu to put the items in and show it
    menu = self.create_default_menu()
    menu.append(prefs_menu)
    menu.append(detach_menu)
    menu.append(about_menu)
    menu.show_all()
    menu.popup(None, None, None, event.button, event.time)
  
  #Detach the applet
  def do_detach(self,*args):
    self.detach.detach()
    self.displayed = False
    self.detached = True
  
  #The applet was detached
  #Do NOT hide the icon; hide the dialog (just in case)
  def was_detached(self):
    self.detached = True
    self.update_icon()
    self.hide_dialog()
  
  #Show the preferences menu
  def show_prefs(self,*args):
    import prefs
    prefs.Prefs(self.settings)
  
  #Attach the applet
  def do_attach(self,*args):
    self.detach.attach()
  
  #The applet was attached
  def was_attached(self,*args):
    #Show the regular icon
    self.detached = False
    self.last_num_items = -1
    self.update_icon()
  
  #Show the about dialog
  def show_about(self,*args):
    win = gtk.AboutDialog()
    win.set_name("To-Do List")
    win.set_copyright("Copyright 2008 sharkbaitbobby "+\
      "<sharkbaitbobby+awn@gmail.com>")
    win.set_authors(["sharkbaitbobby <sharkbaitbobby+awn@gmail.com>"])
    win.set_comments("A simple To-Do List")
    win.set_license("This program is free software; you can redistribute it "+\
      "and/or modify it under the terms of the GNU General Public License "+\
      "as published by the Free Software Foundation; either version 2 of "+\
      "the License, or (at your option) any later version. This program is "+\
      "distributed in the hope that it will be useful, but WITHOUT ANY "+\
      "WARRANTY; without even the implied warranty of MERCHANTABILITY or "+\
      "FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public "+\
      "License for more details. You should have received a copy of the GNU "+\
      "General Public License along with this program; if not, write to the "+\
      "Free Software Foundation, Inc.,"+\
      "51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA.")
    win.set_wrap_license(True)
    win.set_documenters(["sharkbaitbobby <sharkbaitbobby+awn@gmail.com>"])
    win.set_artists(["Cairo"])
    win.run()
    win.destroy()
  
  #Hide the icon
  def hide_icon(self):
    self.hide()
  
  #Hide the dialog
  def hide_dialog(self,*args):
    #The dialog is no longer displayed
    self.displayed = False
    
    #Clear the dialog
    self.clear_dialog()
    
    #Hide the Awn Dialog if necessary
    self.dialog.hide()
  
  #Attached:
  #  Displayed: Hide
  #  Otherwide: Show
  #Detached:
  #  Not displayed: Show
  #  Displayed: This won't be called
  def toggle_dialog(self,*args):
    if self.detached==False and self.displayed==True:
      self.hide_dialog()
    
    elif (self.detached==True and self.displayed==False) or\
      self.detached==False:
      #Make the dialog
      self.make_dialog()
      
      #Deal with the dialog as appropriate
      if self.detached==False:
        self.dialog.show_all()
        if self.settings['title'] in [None, 'To-Do List']:
          self.dialog.set_title('')
        else:
          self.dialog.set_title(self.settings['title'])
      else:
        self.dialog_widget.show_all()
      
      #Fix the first item selected bug (?)
      try:
        self.dialog_widgets[0][1].select_region(1,2)
        self.dialog_widgets[0][1].set_position(0)
      except:
        pass
      
      #Give the Add button focus
      self.dialog_add.grab_focus()
      self.displayed = True
  
  #Show the dialog - detached only
  def show_dialog(self,*args):
    if self.detached==True and self.detach['displayed']==False:
      self.make_dialog()
      self.dialog_widget.show_all()
  
  #Make the dialog - don't show it
  def make_dialog(self,*args):
    #Remove any previous dialog widgets
    self.clear_dialog()
    
    self.dialog_widgets = []
    
    
    #Make the main table
    dialog_table = gtk.Table(1,1)
    
    #Go through the list of to-do items
    y = 0
    for x in self.settings['items']:
      if x!='':
        #This is a normal item
        #Make an "X" button to clear the item
        dialog_x = gtk.Button()
        dialog_x.set_tooltip_text('Remove item')
        dialog_x_icon = gtk.image_new_from_pixbuf(\
          self.icon_theme.load_icon('list-remove',16,16))
        dialog_x.set_image(dialog_x_icon)
        dialog_x.set_relief(gtk.RELIEF_NONE)
        dialog_x.iterator = y
        dialog_x.connect('clicked',self.remove_item)
        
        #Make an entry widget for the item
        dialog_entry = gtk.Entry()
        dialog_entry.set_text(x)
        dialog_entry.iterator = y
        dialog_entry.type = 'items'
        if self.settings['details'][y].replace(' ','').replace('\n','') != '':
          dialog_entry.set_tooltip_text(self.settings['details'][y])
        if self.settings['use_custom_width']:
          if self.settings['custom_width'] >= 25 and \
            self.settings['custom_width'] <= 500:
            dialog_entry.set_size_request(self.settings['custom_width'], -1)
        dialog_entry.connect('focus-out-event',self.item_updated)
        
        
        #Try to colorize the entry widget based on its priority
        try:
          #High: Red
          if self.settings['priority'][y]==3:
            dialog_entry.modify_base(\
              gtk.STATE_NORMAL,gtk.gdk.color_parse(self.settings['color_high']))
            dialog_entry.modify_text(\
              gtk.STATE_NORMAL,gtk.gdk.color_parse( \
              self.settings['color_high_text']))
          #Medium: Yellow
          elif self.settings['priority'][y]==2:
            dialog_entry.modify_base(\
              gtk.STATE_NORMAL,gtk.gdk.color_parse(self.settings['color_med']))
            dialog_entry.modify_text(\
              gtk.STATE_NORMAL,gtk.gdk.color_parse( \
              self.settings['color_med_text']))
          #Low: Green
          elif self.settings['priority'][y]==1:
            dialog_entry.modify_base(\
              gtk.STATE_NORMAL,gtk.gdk.color_parse(self.settings['color_low']))
            dialog_entry.modify_text(\
              gtk.STATE_NORMAL,gtk.gdk.color_parse(\
              self.settings['color_low_text']))
        
        except:
          pass
        
        #Make a ProgressButton
        dialog_progress = ProgressButton(self, y)
        
        #Put the widgets in the table
        if self.settings['category'][y]!=-1:
          dialog_table.attach(dialog_x,0,1,y,(y+1),\
            xoptions=gtk.SHRINK,yoptions=gtk.SHRINK)
          dialog_table.attach(dialog_entry,2,3,y,(y+1),\
            yoptions=gtk.SHRINK)
          dialog_table.attach(dialog_progress, 3, 4, y, (y+1), \
            xoptions=gtk.SHRINK, yoptions=gtk.SHRINK)
        else:
          dialog_table.attach(dialog_x,0,1,y,(y+1),\
            xoptions=gtk.SHRINK,yoptions=gtk.SHRINK)
          dialog_table.attach(dialog_entry,2,3,y,(y+1),\
            yoptions=gtk.SHRINK)
          dialog_table.attach(dialog_progress, 3, 4, y, (y+1), \
            xoptions=gtk.SHRINK, yoptions=gtk.SHRINK)
        
        
        #Put the widgets in a list of widgets - used for expanding categories
        self.dialog_widgets.append([dialog_x,dialog_entry,dialog_progress])
        self.progress_buttons.append(dialog_progress)
        
        #If this item is in a category - don't show it automatically (show_all)
        if self.settings['category'][y] not in [-1]+self.settings['expanded']:
          dialog_x.set_no_show_all(True)
          dialog_entry.set_no_show_all(True)
          dialog_progress.set_no_show_all(True)
        
        y+=1
      
      #This is a category - show an Expander widget
      else:
        #Make a normal X button
        dialog_x = gtk.Button()
        dialog_x.set_tooltip_text('Remove category')
        dialog_x_icon = gtk.image_new_from_pixbuf(\
          self.icon_theme.load_icon('list-remove',16,16))
        dialog_x.set_image(dialog_x_icon)
        dialog_x.set_relief(gtk.RELIEF_NONE)
        dialog_x.iterator = y
        dialog_x.connect('clicked',self.remove_item)
        
        #Make the Expander widget
        dialog_expander = gtk.Expander(self.settings['category_name'][y])
        if self.settings['details'][y].replace(' ','').replace('\n','') != '':
          dialog_expander.set_tooltip_text(self.settings['details'][y])
        if self.settings['use_custom_width']:
          if self.settings['custom_width'] >= 25 and \
            self.settings['custom_width'] <= 500:
            dialog_expander.set_size_request(self.settings['custom_width'], -1)
        dialog_expander.iterator = y
        if y in self.settings['expanded']:
          dialog_expander.set_expanded(True)
        dialog_expander.connect('notify::expanded',self.expanded)
        
        #Make a normal -> button - but different function
        #for the category
        dialog_details = gtk.Button()
        dialog_details.set_tooltip_text('View/Edit details')
        dialog_details_icon = gtk.image_new_from_pixbuf(\
          self.icon_theme.load_icon('go-next',16,16))
        dialog_details.set_image(dialog_details_icon)
        dialog_details.set_relief(gtk.RELIEF_NONE)
        dialog_details.iterator = y
        dialog_details.connect('clicked',self.category_details)
        
        #Now figure out how many items are in this category
        num_items = 0
        for x in self.settings['category']:
          if x == y:
            num_items += 1
        
        #Now make a vertical separator and add it to the dialog
        dialog_vsep = gtk.VSeparator()
        if y not in self.settings['expanded']:
          dialog_vsep.set_no_show_all(True)
        
        #Put the widgets in the table
        dialog_table.attach(dialog_x,0,1,y,(y+1),\
          xoptions=gtk.SHRINK,yoptions=gtk.SHRINK)
        dialog_table.attach(dialog_expander,1,3,y,(y+1),\
          yoptions=gtk.SHRINK)
        dialog_table.attach(dialog_details,3,4,y,(y+1),\
          xoptions=gtk.SHRINK,yoptions=gtk.SHRINK)
        if num_items > 0:
          dialog_table.attach(dialog_vsep,1,2,y+1,(y+1+num_items),\
            xoptions=gtk.SHRINK,xpadding=7)
        
        #Put the widgets in a list of widgets - used for expanding categories
        self.dialog_widgets.append([dialog_x,dialog_expander,dialog_details,\
          dialog_vsep])
        
        y+=1
    
    #Make a button to display a dialog to add an item
    self.dialog_add = gtk.Button(stock=gtk.STOCK_ADD)
    self.dialog_add.connect('clicked',self.add_item)
    
    if len(self.settings['items'])==0:
      #If # items is 0, make the button take up all the width
      dialog_table.attach(self.dialog_add,0,5,y,(y+1),\
        yoptions=gtk.SHRINK)
    
    else:
      #Otherwise, just add it normally; center aligned
      dialog_table.attach(self.dialog_add,0,5,y,(y+1),\
        xoptions=gtk.SHRINK,yoptions=gtk.SHRINK)
    
    #Put the table in the dialog
    dialog_table.show_all()
    self.add_to_dialog(dialog_table)
  
  #Called when an item has been edited by the default dialog
  #(or the edit details dialog)
  def item_updated(self,widget,event):
    if widget.get_text()!='':
      tmp_list_names = []
      y = 0
      for x in self.settings[widget.type]:
        if y!=widget.iterator:
          tmp_list_names.append(x)
        else:
          tmp_list_names.append(widget.get_text())
        y+=1
      self.settings[widget.type] = tmp_list_names
  
  #An Expander widget was expanded or un-expanded
  def expanded(self,widget,expanded):
    
    #Show the category's widgets
    if widget.get_property('expanded'):
      #Show the separator
      self.dialog_widgets[widget.iterator][3].show()
      
      #Find the items that are in this category
      y = 0
      for x in self.settings['category']:
        if x == widget.iterator:
          for x in self.dialog_widgets[y]:
            x.show()
        y+=1
      
      tmp_list_expanded = []
      for x in self.settings['expanded']:
        tmp_list_expanded.append(x)
      tmp_list_expanded.append(widget.iterator)
      self.settings['expanded'] = tmp_list_expanded
    
    #Hide the category's widgets
    else:
      #Hide the separator
      self.dialog_widgets[widget.iterator][3].hide()
      
      #Find the items tat are in this category
      y = 0
      for x in self.settings['category']:
        if x == widget.iterator:
          for x in self.dialog_widgets[y]:
            x.hide()
        y+=1
      
      tmp_list_expanded = []
      for x in self.settings['expanded']:
        tmp_list_expanded.append(x)
      tmp_list_expanded.remove(widget.iterator)
      self.settings['expanded'] = tmp_list_expanded
  
  #Display dialog to add an item to the To-Do list
  def add_item(self, *args):
    #Clear the dialog
    self.clear_dialog()
    
    self.add_category = -1
    self.add_mode = 'to-do'
    
    #Make the main widget - VBox
    self.add_vbox = gtk.VBox()
    
    #Make the RadioButtons for each category
    #First category: No category! (Uncategorized)
    uncategorized = gtk.RadioButton(label='_Uncategorized')
    uncategorized.id = -1
    uncategorized.connect('toggled',self.add_radio_changed)
    self.add_vbox.pack_start(uncategorized,False)
    
    #Now through each category
    y = 0#For each item OR category
    for x in self.settings['category_name']:
      if x!='':
        category = gtk.RadioButton(uncategorized,x)
        category.id = y
        category.connect('toggled',self.add_radio_changed)
        self.add_vbox.pack_start(category,False)
      y+=1
    
    #Simple horizontal separator
    add_hsep = gtk.HSeparator()
    self.add_vbox.pack_start(add_hsep,False,False,3)
    
    #HBox for the two RadioButtons - ( )Category (-)To-Do item
    radio_hbox = gtk.HBox()
    self.add_vbox.pack_start(radio_hbox,False)
    
    #First RadioButton - ( )Category
    category_radio = gtk.RadioButton(label='_Category')
    category_radio.id = 'category'
    category_radio.connect('toggled',self.add_radio_changed)
    radio_hbox.pack_start(category_radio,False)
    
    #Second RadioButton - (-)To-Do item
    #TODO: better text than "To-Do item"?
    item_radio = gtk.RadioButton(category_radio,'_To-Do item')
    item_radio.set_active(True)
    item_radio.id = 'to-do'
    item_radio.connect('toggled',self.add_radio_changed)
    radio_hbox.pack_end(item_radio,False)
    
    #HBox for the entry and button widgets
    add_hbox = gtk.HBox()
    self.add_vbox.pack_start(add_hbox,False)
    
    #Entry for the name
    self.add_entry = gtk.Entry()
    self.add_entry.connect('key-press-event',self.key_press_event,\
      self.add_item_to_list)
    add_hbox.pack_start(self.add_entry)
    if self.settings['use_custom_width']:
      if self.settings['custom_width'] >= 25 and \
        self.settings['custom_width'] <= 500:
        self.add_entry.set_size_request(self.settings['custom_width'], -1)
    
    #OK Button
    add_button = gtk.Button(stock=gtk.STOCK_OK)
    add_button.connect('clicked',self.add_item_to_list)
    add_hbox.pack_start(add_button,False)
    
    #Put it all together
    self.add_vbox.show_all()
    self.add_to_dialog(self.add_vbox)
    self.add_entry.grab_focus()
  
  #When a RadioButton is toggled
  #Either a category radio OR the "Category" or "To-Do item" radios
  def add_radio_changed(self,button):
    if button.get_active()==True:
      #New item will be a category
      if button.id == 'category':
        self.add_mode = 'category'
        for x in self.add_vbox.get_children()[:-2]:
          x.hide()
      #New item will be a normal to-do item
      elif button.id == 'to-do':
        self.add_mode = 'to-do'
        for x in self.add_vbox.get_children()[:-2]:
          x.show()
      #A specific category was selected
      else:
        self.add_category = button.id
  
  #When a key is pressed on a connected entry widget
  #checks for enter key pressed and calls a passed function
  #if the enter key is pressed
  def key_press_event(self,widget,event,func,*args):
    if event.keyval in [65293,65421] or event.hardware_keycode in [36,108]:
      func(*args)
  
  #Edit the details of an item
  def edit_details(self, num, edit_progress=False):
    if type(num)==gtk.Button:
      num = num.iterator
    
    #Get data
    name = self.settings['items'][num]
    priority = self.settings['priority'][num]
    progress = self.settings['progress'][num]
    details = self.settings['details'][num]
    
    #Main widget
    widget = gtk.VBox()
    
    #Name Entry
    name_entry = gtk.Entry()
    name_entry.set_text(name)
    name_entry.iterator = num
    name_entry.type = 'items'
    if self.settings['use_custom_width']:
      if self.settings['custom_width'] >= 25 and \
        self.settings['custom_width'] <= 500:
        name_entry.set_size_request(self.settings['custom_width'], -1)
    name_entry.connect('focus-out-event',self.item_updated)
    
    #HBoxes for Priority Label and RadioButtons
    priority_hbox0 = gtk.HBox()
    priority_hbox1 = gtk.HBox()
    
    #Label: Priority: 
    priority_label = gtk.Label('Priority: ')
    
    #Neutral, Low, Medium, and High priority RadioButtons
    priority_neutral = gtk.RadioButton(label='_Neutral')
    priority_neutral.id = [0,num]
    priority_low = gtk.RadioButton(priority_neutral,'_Low')
    priority_low.id = [1,num]
    priority_med = gtk.RadioButton(priority_neutral,'_Medium')
    priority_med.id = [2,num]
    priority_high = gtk.RadioButton(priority_neutral,'_High')
    priority_high.id = [3,num]
    
    #Select the right RadioButton (Neutral is selected by default)
    if priority==1:
      priority_low.set_active(True)
    elif priority==2:
      priority_med.set_active(True)
    elif priority==3:
      priority_high.set_active(True)
    
    #Connect the radio buttons to the radio_selected function
    priority_neutral.connect('toggled',self.radio_selected)
    priority_low.connect('toggled',self.radio_selected)
    priority_med.connect('toggled',self.radio_selected)
    priority_high.connect('toggled',self.radio_selected)
    
    #Pack the widgets to the HBoxes
    priority_hbox0.pack_start(priority_label)
    priority_hbox0.pack_start(priority_neutral,False)
    priority_hbox1.pack_start(priority_low,True,False)
    priority_hbox1.pack_start(priority_med,True,False)
    priority_hbox1.pack_start(priority_high,True,False)
    
    #HBox for Progress label and SpinButton
    progress_hbox = gtk.HBox()
    
    #Label: Progress(%): 
    progress_label = gtk.Label('Progress(%): ')
    
    #SpinButton and Adjustment for the SpinButton
    progress_adj = gtk.Adjustment(float(progress),0,100,5,10,1)
    progress_spin = gtk.SpinButton(progress_adj,1,0)
    progress_spin.iterator = num
    progress_spin.connect('focus-out-event',self.spin_focusout)
    if edit_progress == True:
      progress_spin.grab_focus()
    
    #Pack the widgets to the HBox
    progress_hbox.pack_start(progress_label,False)
    progress_hbox.pack_start(progress_spin)
    
    #Make a TextView to edit the details of the the item
    details_scrolled = gtk.ScrolledWindow()
    details_scrolled.set_policy(gtk.POLICY_AUTOMATIC,gtk.POLICY_AUTOMATIC)
    details_textbuffer = gtk.TextBuffer()
    details_textbuffer.set_text(details)
    details_textview = gtk.TextView(details_textbuffer)
    details_textview.set_wrap_mode(gtk.WRAP_WORD)
    details_textview.iterator = num
    details_textview.connect('focus-out-event',self.textview_focusout)
    details_scrolled.add_with_viewport(details_textview)
    details_scrolled.set_size_request(0,100)
    
    #Simple "OK" button - display the main dialog
    ok_button = gtk.Button(stock=gtk.STOCK_OK)
    ok_button.connect('clicked',self.make_dialog)
    
    #Pack the widgets to the main VBox
    widget.pack_start(name_entry,False)
    widget.pack_start(priority_hbox0,False)
    widget.pack_start(priority_hbox1,False)
    widget.pack_start(progress_hbox,False)
    widget.pack_start(details_scrolled,True,True,3)
    widget.pack_start(ok_button,False)
    
    #Put everything together
    widget.show_all()
    self.clear_dialog()
    self.add_to_dialog(widget)
    if not edit_progress:
      ok_button.grab_focus()
  
  #When a RadioButton from the edit details dialog has been selected
  def radio_selected(self,widget):
    if widget.get_active()==True:
      tmp_list_priority = []
      y = 0
      for x in self.settings['priority']:
        if y==widget.id[1]:
          tmp_list_priority.append(widget.id[0])
        else:
          tmp_list_priority.append(x)
        y+=1
      self.settings['priority'] = tmp_list_priority
  
  #When the SpinButton for progress has lost focus
  def spin_focusout(self,widget,event):
    tmp_list_progress = []
    y = 0
    for x in self.settings['progress']:
      if y==widget.iterator:
        tmp_list_progress.append(widget.get_value())
      else:
        tmp_list_progress.append(x)
      y+=1
    self.settings['progress'] = tmp_list_progress
  
  #When the TextView for details has lost focus
  def textview_focusout(self,widget,event):
    tmp_list_details = []
    y = 0
    for x in self.settings['details']:
      if y==widget.iterator:
        tmp_list_details.append(widget.get_buffer().get_text(\
          widget.get_buffer().get_start_iter(),\
          widget.get_buffer().get_end_iter(),False))
      else:
        tmp_list_details.append(x)
      y+=1
    self.settings['details'] = tmp_list_details
  
  #Edit the details of a category
  def category_details(self,catid):
    if type(catid) != int:#Could be GtkButton
      catid = catid.iterator
    
    #Get data
    name = self.settings['category_name'][catid]
    details = self.settings['details'][catid]
    
    #Main widget
    widget = gtk.VBox()
    
    #Name Entry
    name_entry = gtk.Entry()
    name_entry.set_text(name)
    name_entry.iterator = catid
    name_entry.type = 'category_name'
    name_entry.connect('focus-out-event',self.item_updated)
    if self.settings['use_custom_width']:
      if self.settings['custom_width'] >= 25 and \
        self.settings['custom_width'] <= 500:
        name_entry.set_size_request(self.settings['custom_width'], -1)
    
    #Make a TextView to edit the details of the the item
    details_scrolled = gtk.ScrolledWindow()
    details_scrolled.set_policy(gtk.POLICY_AUTOMATIC,gtk.POLICY_AUTOMATIC)
    details_textbuffer = gtk.TextBuffer()
    details_textbuffer.set_text(details)
    details_textview = gtk.TextView(details_textbuffer)
    details_textview.set_wrap_mode(gtk.WRAP_WORD)
    details_textview.iterator = catid
    details_textview.connect('focus-out-event',self.textview_focusout)
    details_scrolled.add_with_viewport(details_textview)
    details_scrolled.set_size_request(0,100)
    
    #Simple "OK" button - displays the main dialog
    ok_button = gtk.Button(stock=gtk.STOCK_OK)
    ok_button.connect('clicked',self.make_dialog)
    
    #Pack the widgets to the main VBox
    widget.pack_start(name_entry,False)
    widget.pack_start(details_scrolled,True,True,3)
    widget.pack_start(ok_button,False)
    
    #Put everything together
    widget.show_all()
    self.clear_dialog()
    self.add_to_dialog(widget)
    ok_button.grab_focus()
  
  #Called when the list of items has been changed - change the icon
  def update_icon(self,*args):
    if self.last_num_items!=len(self.settings['items']) or \
      self.settings['icon-type'] in ['progress','progress-items'] or \
      (len(args) > 0 and args[0] in ['color', 'colors', 'icon-type']):
      
      #Update the icon colors
      if self.settings['color'] in ['butter','chameleon','orange','skyblue',\
        'plum','chocolate','scarletred','aluminium1','aluminium2']:
        self.color = icon.colors[self.settings['color']]
      elif self.settings['color'] == 'custom':
        self.update_custom_colors()
      elif self.settings['color'] == 'gtk':
        self.update_icon_theme()
      else:
        self.settings['color'] = 'skyblue'
        self.color = icon.colors['skyblue']
      
      #Get the number of items, excluding categories
      tmp_items = []
      for item in self.settings['items']:
        if item != '':
          tmp_items.append(item)
        
      #Change the detached icon first
      try:
        assert len(tmp_items) == 0
        self.detach['icon-mode'] = 'pixbuf'
        self.detach.set_pixbuf(self.icon_theme.load_icon(\
          'view-sort-descending',self.height,self.height))
      except:
        self.detach['icon-mode'] = 'surface'
        self.detach.set_surface(icon.icon(48, self.settings, self.color, \
          self.surface, self.last_height))
      
      #Change the attached applet icon second
      if self.detached == False:
        self.show_all()
      
        try:
          assert len(tmp_items) == 0
          self.set_icon(self.icon_theme.load_icon(\
            'view-sort-descending',self.height,self.height))
        except:
          
          #If Awn supports setting the icon as a cairo context
          self.surface = icon.icon(self.height, self.settings, self.color, \
            self.surface, self.last_height)
          self.context = cairo.Context(self.surface)
          self.set_icon_context(self.context)
      
      self.last_height = self.height
      self.last_num_items = len(self.settings['items'])
  
  #Update the colors for the icon if the current icon theme
  #if the current GTK theme ('gtk')
  #Does NOT update the icon
  def update_icon_theme(self):
    if self.settings['color'] == 'gtk':
      #Get the colors from a temporary window
      self.color = [None,None,None,None]
      tmp_window = gtk.Window()
      tmp_window.realize()
      
      #Outer and inner borders - rgb
      tmp_innerborder = tmp_window.get_style().bg[gtk.STATE_SELECTED]
      self.color[2] = [tmp_innerborder.red/256.0,\
        tmp_innerborder.green/256.0,tmp_innerborder.blue/256.0]
      self.color[0] = [tmp_innerborder.red/256.0,\
        tmp_innerborder.green/256.0,tmp_innerborder.blue/256.0]
      
      #Main color - rgb
      tmp_maincolor = tmp_window.get_style().bg[gtk.STATE_NORMAL]
      self.color[1] = [tmp_maincolor.red/256.0,\
        tmp_maincolor.green/256.0,tmp_maincolor.blue/256.0]
      
      #Text color - rgb
      tmp_textcolor = tmp_window.get_style().text[gtk.STATE_PRELIGHT]
      self.color[3] = [tmp_textcolor.red/256.0,\
        tmp_textcolor.green/256.0,tmp_textcolor.blue/256.0]
      
      #Save some memory (is this necessary?)
      tmp_window.destroy()
      #Get the colors of the custom icon color
  
  def update_custom_colors(self):
    self.color = []
    
    #Check if the list of colors is set
    if len(self.settings['colors']) < 12:
      self.settings['colors'] = [255, 255, 255, 127, 127, 127, 0, 0, 0, \
        255, 255, 255]
    
    #Inner border - red, green, blue
    self.color.append([self.settings['colors'][3],self.settings['colors'][4],\
      self.settings['colors'][5]])
    
    #Main color - red, green, blue
    self.color.append([self.settings['colors'][6],self.settings['colors'][7],\
      self.settings['colors'][8]])
    
    #Outer border - red, green, blue
    self.color.append([self.settings['colors'][0],self.settings['colors'][1],\
      self.settings['colors'][2]])
    
    #Text color - reg, green, blue
    self.color.append([self.settings['colors'][9],self.settings['colors'][10],\
      self.settings['colors'][11]])
  
  #Change the opacity of the icon by 5%
  def opacity(self,event,more):
    old_opacity = self.settings['icon-opacity']
    new_opacity = False
    
    #Increase opacity
    if more:
      
      #Make sure it won't go too far
      if old_opacity + 5 >= 100:
        new_opacity = 100
      #Increase by 5%
      else:
        new_opacity = old_opacity + 5
    
    #Decrease opacity
    else:
      
      #Make sure it won't go too far
      if old_opacity - 5 <= 10:
        new_opacity = 10
      #Decrease by 5%
      else:
        new_opacity = old_opacity - 5
    
    #Update the icon if necessary
    if old_opacity != new_opacity:
      self.settings['icon-opacity'] = new_opacity
      self.last_num_items = -1
      self.update_icon()
  
  #Actually add the item to the list of items
  def add_item_to_list(self,*args):
    #Make sure that the item name is not empty
    if self.add_entry.get_text().replace(' ','')!='':
      #Find out what to do based on category things
      if self.add_mode == 'to-do':
        if self.add_category == -1:
          #Just append
          tmp_list_names = []
          tmp_list_priority = []
          tmp_list_progress = []
          tmp_list_details = []
          tmp_list_category = []
          tmp_list_category_name = []
          for x in self.settings['items']:
            tmp_list_names.append(x)
          for x in self.settings['priority']:
            tmp_list_priority.append(x)
          for x in self.settings['progress']:
            tmp_list_progress.append(x)
          for x in self.settings['details']:
            tmp_list_details.append(x)
          for x in self.settings['category']:
            tmp_list_category.append(x)
          for x in self.settings['category_name']:
            tmp_list_category_name.append(x)
          
          
          tmp_list_names.append(self.add_entry.get_text())
          tmp_list_priority.append(0)
          tmp_list_progress.append(0)
          tmp_list_details.append('')
          tmp_list_category.append(-1)
          tmp_list_category_name.append('')
          
          self.settings['items'] = tmp_list_names
          self.settings['priority'] = tmp_list_priority
          self.settings['progress'] = tmp_list_progress
          self.settings['details'] = tmp_list_details
          self.settings['category'] = tmp_list_category
          self.settings['category_name'] = tmp_list_category_name
          
          #Re-show the main dialog
          self.displayed = False#Is this necessary?
          self.clear_dialog()
          self.edit_details(len(tmp_list_names)-1)
        
        #A category was selected; add this item to the end of that category!
        else:
          #Find where to put the new item
          where = -1
          y = 0
          for x in self.settings['category']:
            if x == self.add_category:
              where = y
            y += 1
          
          if where==-1:
            #This means that there are no items in the category
            where = self.add_category
          
          tmp_list_names = []
          tmp_list_priority = []
          tmp_list_progress = []
          tmp_list_details = []
          tmp_list_category = []
          tmp_list_category_name = []
          
          y = 0
          for x in self.settings['items']:
            tmp_list_names.append(x)
            if y == where:
              tmp_list_names.append(self.add_entry.get_text())
            y+=1
          
          y = 0
          for x in self.settings['priority']:
            tmp_list_priority.append(x)
            if y == where:
              tmp_list_priority.append(0)
            y+=1
          
          y = 0
          for x in self.settings['progress']:
            tmp_list_progress.append(x)
            if y == where:
              tmp_list_progress.append(0)
            y+=1
          
          y = 0
          for x in self.settings['details']:
            tmp_list_details.append(x)
            if y == where:
              tmp_list_details.append('')
            y+=1
          
          y = 0
          for cat in self.settings['category']:
            if y > where and cat != -1:
              tmp_list_category.append(cat+1)
            else:
              tmp_list_category.append(cat)
            
            if y == where:
              tmp_list_category.append(self.add_category)
            y+=1
          
          y = 0
          for x in self.settings['category_name']:
            tmp_list_category_name.append(x)
            if y == where:
              tmp_list_category_name.append('')
            y+=1
          
          self.settings['items'] = tmp_list_names
          self.settings['priority'] = tmp_list_priority
          self.settings['progress'] = tmp_list_progress
          self.settings['details'] = tmp_list_details
          self.settings['category'] = tmp_list_category
          self.settings['category_name'] = tmp_list_category_name
          
          #Re-show the main dialog
          self.displayed = False#TODO:Is this necessary?
          self.clear_dialog()
          self.edit_details(where+1)
      
      #The new item is a category
      else:
        #Append to the list
        tmp_list_names = []
        tmp_list_priority = []
        tmp_list_progress = []
        tmp_list_details = []
        tmp_list_category = []
        tmp_list_category_name = []
        for x in self.settings['items']:
          tmp_list_names.append(x)
        for x in self.settings['priority']:
          tmp_list_priority.append(x)
        for x in self.settings['progress']:
          tmp_list_progress.append(x)
        for x in self.settings['details']:
          tmp_list_details.append(x)
        for x in self.settings['category']:
          tmp_list_category.append(x)
        for x in self.settings['category_name']:
          tmp_list_category_name.append(x)
        
        
        tmp_list_names.append('')
        tmp_list_priority.append(0)
        tmp_list_progress.append(0)
        tmp_list_details.append('')
        tmp_list_category.append(-1)
        tmp_list_category_name.append(self.add_entry.get_text())
        
        self.settings['items'] = tmp_list_names
        self.settings['priority'] = tmp_list_priority
        self.settings['progress'] = tmp_list_progress
        self.settings['details'] = tmp_list_details
        self.settings['category'] = tmp_list_category
        self.settings['category_name'] = tmp_list_category_name
        
        #Re-show the main dialog
        self.displayed = False#TODO:Is this necessary?
        self.clear_dialog()
        self.category_details(len(tmp_list_names)-1)
    
    #The item name is empty; display the main dialog
    else:
      self.displayed = False
      self.toggle_dialog()
  
  #Display a confirmation dialog about removing an item/category from the list
  #(If appropriate)
  def remove_item(self,itemid):
    if type(itemid)!=int:
      itemid = itemid.iterator
    
    #If the 'item' is a category
    if self.settings['items'][itemid] == '':
      #If user wants to be warned
      if self.settings['confirm-categories'] == True:
        
        #Make Label
        confirm_label = gtk.Label('Are you sure you want to remove the ' + \
          'category "%s?"\nAll of its items will be removed.' % \
          self.settings['category_name'][itemid])
        
        #Make CheckButton
        confirm_check = gtk.CheckButton('Don\'t show this again.')
        confirm_check.key = 'confirm-categories'
        confirm_check.connect('toggled', self.confirm_check)
        
        #Cancel and OK buttons
        cancel_button = gtk.Button(stock=gtk.STOCK_CANCEL)
        cancel_button.connect('clicked', self.make_dialog)
        remove_button = gtk.Button(stock=gtk.STOCK_REMOVE)
        remove_button.iterator = itemid
        remove_button.connect('clicked', self.remove_item_from_list)
        
        #Now put it all together
        vbox = gtk.VBox()
        hbbox = gtk.HButtonBox()
        hbbox.set_layout(gtk.BUTTONBOX_END)
        hbbox.pack_start(cancel_button, padding=12)
        hbbox.pack_start(remove_button)
        vbox.pack_start(confirm_label, False)
        vbox.pack_start(confirm_check, False)
        vbox.pack_start(hbbox, False)
        vbox.show_all()
        self.clear_dialog()
        self.add_to_dialog(vbox)
      
      #User does not want to be warned
      else:
        self.remove_item_from_list(itemid)
    
    #deletion-prone item is deletion-prone
    else:
      #If user wants to be warned
      if self.settings['confirm-items'] == True:
        
        #Make Label
        confirm_label = gtk.Label('Are you sure you want to remove this item?')
        
        #Make CheckButton
        confirm_check = gtk.CheckButton('Don\'t show this again.')
        confirm_check.key = 'confirm-items'
        confirm_check.connect('toggled', self.confirm_check)
        
        #Cancel and OK buttons
        cancel_button = gtk.Button(stock=gtk.STOCK_CANCEL)
        cancel_button.connect('clicked', self.make_dialog)
        remove_button = gtk.Button(stock=gtk.STOCK_REMOVE)
        remove_button.iterator = itemid
        remove_button.connect('clicked', self.remove_item_from_list)
        
        #Now put it all together
        vbox = gtk.VBox()
        hbbox = gtk.HButtonBox()
        hbbox.set_layout(gtk.BUTTONBOX_END)
        hbbox.pack_start(cancel_button, padding=12)
        hbbox.pack_start(remove_button)
        vbox.pack_start(confirm_label, False)
        vbox.pack_start(confirm_check, False)
        vbox.pack_start(hbbox, False)
        vbox.show_all()
        self.clear_dialog()
        self.add_to_dialog(vbox)
      
      #User does not want to be warned
      else:
        self.remove_item_from_list(itemid)
  
  #Remove an item from the list of items
  def remove_item_from_list(self,itemid):
    
    if type(itemid)!=int:
      itemid = itemid.iterator
    
    #List of items in this category
    list_of_items = [itemid]
    
    #If this is a category and it has items in it,
    #remove its items first
    if self.settings['items'][itemid]=='':#Means it's a category
      #Remove this category's items
      y = 0
      for x in self.settings['category']:
        if x == itemid:
          list_of_items.append(y)
        y+=1
      
      #Remove this category from the list of expanded categories
      if itemid in self.settings['expanded']:
        tmp_list_expanded = self.settings['expanded']
        tmp_list_expanded.remove(itemid)
        self.settings['expanded'] = tmp_list_expanded
    
    tmp_list_names = []
    tmp_list_priority = []
    tmp_list_progress = []
    tmp_list_details = []
    tmp_list_category = []
    tmp_list_category_name = []
    tmp_list_expanded = []
    
    y = 0
    for x in self.settings['items']:
      if y not in list_of_items:
        tmp_list_names.append(x)
      y+=1
    
    y = 0
    for x in self.settings['priority']:
      if y not in list_of_items:
        tmp_list_priority.append(x)
      y+=1
    
    y = 0
    for x in self.settings['progress']:
      if y not in list_of_items:
        tmp_list_progress.append(x)
      y+=1
    
    y = 0
    for x in self.settings['details']:
      if y not in list_of_items:
        tmp_list_details.append(x)
      y+=1
    
    #Please, for your own sake, do not write code like this next section.
    i = 0
    for item_category in self.settings['category']:
      if i not in list_of_items:
        #print '%s not in list_of_items:' % i
        if i > list_of_items[-1] and item_category != -1:
          #print 'i > last item (%s) and item_category != -1; i == %s, ' % \
          #  (list_of_items[-1], i)
          if len(list_of_items) == 1:
            #print 'not removing a category'
            #Check if the iterated item is in a category lower than or equal to
            #the item being removed's category (lower = lower in list, higher #)
            if item_category <= self.settings['category'][list_of_items[0]]:
              #It is; just append normally
              #print 'In same or previous category'
              tmp_list_category.append(item_category)
            else:
              #print 'In different category'
              tmp_list_category.append(item_category-1)
          else:
            #print 'removing a category'
            tmp_list_category.append(item_category-len(list_of_items))
        else:
          #print 'normal append to category list'
          tmp_list_category.append(item_category)
        #print
      i+=1
    
    y = 0
    for x in self.settings['category_name']:
      if y not in list_of_items:
        tmp_list_category_name.append(x)
      y+=1
    
    y = 0
    for cat in self.settings['expanded']:
      if cat >= self.settings['category'][list_of_items[-1]]:
        tmp_list_expanded.append(cat-len(list_of_items))
      else:
        tmp_list_expanded.append(cat)
      y += 1
    
    self.settings['items'] = tmp_list_names
    self.settings['priority'] = tmp_list_priority
    self.settings['progress'] = tmp_list_progress
    self.settings['details'] = tmp_list_details
    self.settings['category'] = tmp_list_category
    self.settings['category_name'] = tmp_list_category_name
    self.settings['expanded'] = tmp_list_expanded
    
    #The icon is automatically changed, but the dialog is not
    self.displayed = False
    self.toggle_dialog()
  
  #When a CheckButton for "Don't show this again." is toggled
  def confirm_check(self, button):
    self.settings[button.key] = not button.get_active()

  #Show the title on hover
  def show_title(self, *args):
    if self.settings['title'] is None:
      self.title.show(self, "To-Do List")

    else:
      self.title.show(self, self.settings['title'])

#A gtk.Button that displays and changes an item's progress
class ProgressButton(gtk.Button):
  def __init__(self, applet, Id):
    self.applet = applet
    self.settings = applet.settings
    self.Id = Id
    gtk.Button.__init__(self)
    
    self.surface = None
    self.pixbuf = None
    self.conn = None
    self.conn2 = None
  
    #Set up the icon
    progress = self.settings['progress'][self.Id]
    self.surface = icon.icon2(self.settings, applet.color, self.surface, \
      progress)
    self.pixbuf = surface_to_pixbuf(self.surface)
    image = gtk.image_new_from_pixbuf(self.pixbuf)
    self.set_image(image)
    
    #For looks
    self.set_relief(gtk.RELIEF_NONE)
    
    #Connect to changes in color or items' progress
    self.settings.connect('color', self.update)
    
    #Connect to the scroll event
    self.conn = self.connect('scroll-event', self.scroll)
    self.conn2 = self.connect('clicked', \
      lambda *a: applet.edit_details(self.Id, True))
    
    #Set the tooltip: X% done
    self.set_tooltip_text(str(int(progress)) + '% done')
  
  def update(self, *args):
    #Reset up the icon
    progress = self.settings['progress'][self.Id]
    self.surface = icon.icon2(self.settings, applet.color, self.surface, \
      progress)
    self.pixbuf = surface_to_pixbuf(self.surface)
    image = gtk.image_new_from_pixbuf(self.pixbuf)
    self.set_image(image)
    
    #Reset the tooltip
    self.set_tooltip_text(str(int(progress)) + '% done')
  
  def scroll(self, widget, event):
    #Scrolling up
    if event.direction == gtk.gdk.SCROLL_UP:
      #Increase percentage
      list_of_progress = []
      y = 0
      for progress in self.settings['progress']:
        if y == self.Id:
          if progress + 5 >= 100:
            list_of_progress.append(100)
          else:
            list_of_progress.append(progress + 5)
        
        else:
          list_of_progress.append(progress)
        y += 1
      
      self.settings['progress'] = list_of_progress
      self.update()
    
    #Scrolling down
    elif event.direction == gtk.gdk.SCROLL_DOWN:
      #Decrease percentage
      list_of_progress = []
      y = 0
      for progress in self.settings['progress']:
        if y == self.Id:
          if progress - 5 <= 0:
            list_of_progress.append(0)
          else:
            list_of_progress.append(progress - 5)
        
        else:
          list_of_progress.append(progress)
        y += 1
      
      self.settings['progress'] = list_of_progress
      self.update()
  
  #Why does this not work the way it should???
  def disconn(self):
    self.settings.disconnect('color', self.update)
    for x in dir(self):
      del x
    del self

if __name__ == '__main__':
  awn.init(sys.argv[1:])
  applet = App(awn.uid,awn.orient,awn.height)
  awn.init_applet(applet)
  applet.show_all()
  gtk.main()
