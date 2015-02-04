#! /usr/bin/env python

#
# LibOwonPds
#
# A userspace driver of Owon PDS oscilloscopes
#
# http://eartoearoak.com/software/libowonpds
#
# Copyright 2015 Al Brown
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#

import math
import time
import wx

import libowonpds


class FrameOscilloscope(wx.Frame):
    CHANNEL_COLORS = ['Red', 'Yellow', 'Sky Blue', 'Green']
    REFRESH_RATE = 7

    def __init__(self):
        wx.Frame.__init__(self, None, title='FrameOscilloscope')

        self._crt = PanelCrt(self)

        panel = wx.Panel(self)
        self._controls = PanelControls(panel)
        self._info = PanelInfo(panel)
        self._channels = PanelChannels(panel)
        sizerPanel = wx.BoxSizer(wx.VERTICAL)
        sizerPanel.Add(self._controls, 0, wx.EXPAND)
        sizerPanel.Add(self._info, 0, wx.EXPAND)
        sizerPanel.Add(self._channels, 0, wx.EXPAND)
        panel.SetSizer(sizerPanel)

        self.Bind(wx.EVT_BUTTON, self.__on_control)
        self.Bind(wx.EVT_SPINCTRL, self.__on_control)

        sizer = wx.BoxSizer(wx.HORIZONTAL)
        sizer.Add(self._crt, 1, wx.SHAPED)
        sizer.Add(panel, 0, wx.EXPAND)

        self.SetSizerAndFit(sizer)

        self._scope = None
        self.__open_scope()

        self._refreshPeriod = 1. / FrameOscilloscope.REFRESH_RATE
        self._timer = wx.Timer(self)
        self.Bind(wx.EVT_TIMER, self.__on_timer, self._timer)

        self.Bind(wx.EVT_CLOSE, self.__on_close)

    def __on_close(self, _event):
        self._timer.Stop()
        self._scope.close()
        self.Destroy()

    def __on_control(self, event):
        control = event.GetEventObject()
        if control == self._controls.buttonStart:
            self.__capture_start()
        elif control == self._controls.buttonStop:
            self.__capture_stop()
        elif control == self._controls.spinRate:
            self._refreshPeriod = 1. / self._controls.spinRate.GetValue()
            if self._timer.IsRunning():
                self._timer.Stop()
                self._timer.Start(self._refreshPeriod * 1000, oneShot=True)

    def __on_timer(self, _event):
        self._timer.Stop()
        self.__update()

    def __open_scope(self):
        self._scope = libowonpds.OwonPds()
        if self._scope.open() != 0:
            wx.MessageBox('No scope found', 'Error', wx.OK | wx.ICON_ERROR)
            self.Destroy()
        else:
            self._controls.enable(True, False)

    def __capture_start(self):
        self._controls.enable(False, True)
        self.__update()

    def __capture_stop(self):
        self._controls.enable(True, False)
        self._timer.Stop()

    def __update(self):
        timeStart = time.time()
        if self._scope.read() != 0:
            self.__capture_stop()
            wx.MessageBox('Read failed', 'Error', wx.OK | wx.ICON_ERROR)
        else:
            self._crt.update(self._scope)
            self._info.update(self._scope)
            self._channels.update(self._scope)
            timeRefresh = self._refreshPeriod - (time.time() - timeStart)
            if timeRefresh < 0:
                self._controls.spinRate.SetForegroundColour(wx.RED)
            else:
                self._controls.spinRate.SetForegroundColour(wx.BLACK)
            if timeRefresh < 1e-3:
                timeRefresh = 1e-3

            self._timer.Start(timeRefresh * 1000, oneShot=True)


class PanelCrt(wx.Panel):
    DIV_X = 10
    DIV_Y = 8
    DIV_TICKS = 5
    TICK_LEN = 0.4

    def __init__(self, parent):
        wx.Panel.__init__(self, parent, size=(320, 256),
                          style=wx.FULL_REPAINT_ON_RESIZE | wx.BORDER_RAISED)
        self._scope = None

        try:
            self.SetBackgroundStyle(wx.BG_STYLE_PAINT)
        except AttributeError:
            pass
        self.Bind(wx.EVT_PAINT, self.__on_paint)

    def __on_paint(self, _event):
        dc = wx.AutoBufferedPaintDC(self)

        self.__draw_graticule(dc)
        self.__draw_channels(dc)

    def __draw_graticule(self, dc):
        dc.SetPen(wx.Pen(wx.WHITE, 1))
        dc.SetBrush(wx.BLACK_BRUSH)
        width, height = self.GetSize()
        dc.DrawRectangle(0, 0, width, height)

        scaleX = width / float(PanelCrt.DIV_X * PanelCrt.DIV_TICKS)
        for i in range(PanelCrt.DIV_X * PanelCrt.DIV_TICKS):
            x = i * scaleX
            dc.SetPen(wx.Pen(wx.WHITE, 1))
            tickLen = PanelCrt.TICK_LEN * scaleX
            dc.DrawLine(x, 0, x, tickLen)
            dc.DrawLine(x, height, x, height - tickLen)
            if i % PanelCrt.DIV_TICKS == 0:
                dc.SetPen(wx.Pen(wx.WHITE, 1, wx.DOT))
                dc.DrawLine(x, 0, x, height)

        scaleY = height / float(PanelCrt.DIV_Y * PanelCrt.DIV_TICKS)
        for i in range(PanelCrt.DIV_Y * PanelCrt.DIV_TICKS):
            y = i * scaleY
            dc.SetPen(wx.Pen(wx.WHITE, 1))
            tickLen = PanelCrt.TICK_LEN * scaleY
            dc.DrawLine(0, y, tickLen, y)
            dc.DrawLine(width, y, width - tickLen, y)
            if i % PanelCrt.DIV_TICKS == 0:
                dc.SetPen(wx.Pen(wx.WHITE, 1, wx.DOT))
                dc.DrawLine(0, y, width, y)

    def __draw_channels(self, dc):
        if self._scope is not None:
            data = self._scope.get_scope()
            width, height = self.GetSize()

            for j in range(data.channelCount):
                channel = data.channels[j]
                if channel.samples > 0:
                    scaleX = float(width) / (channel.samples)
                    scaleY = height / float(channel.sensitivity * PanelCrt.DIV_Y)
                    lines = [[] for i in range(channel.samples)]
                    level = channel.vector[0] + channel.offset
                    y = (height / 2) - (level * scaleY)
                    xLast = 0
                    yLast = y
                    for i in range(channel.samples):
                        level = channel.vector[i] + channel.offset
                        y = (height / 2) - (level * scaleY)
                        x = i * scaleX
                        lines[i] = (xLast, yLast, x, y)
                        xLast = x
                        yLast = y
                    colour = FrameOscilloscope.CHANNEL_COLORS[j]
                    dc.SetPen(wx.Pen(colour, 2))
                    dc.DrawLineList(lines)

                    x = channel.slow * channel.sampleRate * scaleX
                    dc.SetPen(wx.Pen(colour, 2))
                    dc.DrawLine(x, 0, x, height)

    def update(self, scope):
        self._scope = scope
        self.Refresh()


class PanelControls(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, style=wx.BORDER_RAISED)
        self.parent = parent

        self.buttonStart = wx.Button(self, label='Start')
        self.buttonStop = wx.Button(self, label='Stop')

        textRate = wx.StaticText(self, label='Refresh rate')
        self.spinRate = wx.SpinCtrl(self, min=1, max=50,
                                    initial=FrameOscilloscope.REFRESH_RATE)
        self.spinRate.SetToolTipString('Turns red if too fast')

        self.enable(False, False)

        self.Bind(wx.EVT_BUTTON, self.__on_control, self.buttonStart)
        self.Bind(wx.EVT_BUTTON, self.__on_control, self.buttonStop)
        self.Bind(wx.EVT_SPINCTRL, self.__on_control, self.spinRate)

        sizerRate = wx.BoxSizer(wx.VERTICAL)
        sizerRate.Add(textRate, 0, wx.ALL, border=5)
        sizerRate.Add(self.spinRate, 0, wx.ALL, border=5)

        sizer = wx.BoxSizer(wx.VERTICAL)
        sizer.Add(self.buttonStart, 0, wx.EXPAND)
        sizer.Add(self.buttonStop, 0, wx.EXPAND)
        sizer.Add(sizerRate, 0, wx.EXPAND)

        self.SetSizerAndFit(sizer)

    def __on_control(self, event):
        wx.PostEvent(self.parent, event)

    def enable(self, start, stop):
        self.buttonStart.Enable(start)
        self.buttonStop.Enable(stop)


class PanelInfo(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, style=wx.BORDER_RAISED)

        textName = wx.StaticText(self, label='Name')
        self._textName = wx.StaticText(self,
                                       style=wx.ALIGN_RIGHT |
                                       wx.ST_NO_AUTORESIZE)
        textType = wx.StaticText(self, label='Type')
        self._textType = wx.StaticText(self,
                                       style=wx.ALIGN_RIGHT |
                                       wx.ST_NO_AUTORESIZE)
        textChannels = wx.StaticText(self, label='Channels')
        self._textChannels = wx.StaticText(self,
                                           style=wx.ALIGN_RIGHT |
                                           wx.ST_NO_AUTORESIZE)

        sizer = wx.GridBagSizer(vgap=4, hgap=4)
        sizer.Add(textName, pos=(0, 0))
        sizer.Add(self._textName, pos=(0, 1), flag=wx.EXPAND)
        sizer.Add(textType, pos=(1, 0))
        sizer.Add(self._textType, pos=(1, 1), flag=wx.EXPAND)
        sizer.Add(textChannels, pos=(2, 0))
        sizer.Add(self._textChannels, pos=(2, 1), flag=wx.EXPAND)
        sizer.AddGrowableCol(1, 1)

        self.update()

        self.SetSizerAndFit(sizer)

    def update(self, scope=None):
        name = ''
        filetype = ''
        channels = ''

        if scope is not None:
            data = scope.get_scope()
            name = '{}'.format(data.name)
            filetype = 'Unknown'
            if data.type == 0:
                filetype = 'Vector'
            elif data.type == 1:
                filetype = 'Bitmap'
            channels = '{}'.format(data.channelCount)

        self._textName.SetLabel(name)
        self._textType.SetLabel(filetype)
        self._textChannels.SetLabel(channels)


class PanelChannels(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, style=wx.BORDER_RAISED)

        sizer = wx.BoxSizer(wx.VERTICAL)
        self._channels = []
        for _i in range(libowonpds.OWON_MAX_CHANNELS):
            channel = PanelChannel(self)
            self._channels.append(channel)
            sizer.Add(channel, 0, wx.EXPAND)

        self.update()

        self.SetSizerAndFit(sizer)

    def update(self, scope=None):
        for i in range(libowonpds.OWON_MAX_CHANNELS):
            channelCount = 0
            if scope is not None:
                data = scope.get_scope()
                channelCount = data.channelCount
            if i < channelCount:
                self._channels[i].update(data.channels[i])
                self._channels[i].Enable()
            else:
                self._channels[i].update(None)
                self._channels[i].Disable()


class PanelChannel(wx.Panel):
    def __init__(self, parent):
        wx.Panel.__init__(self, parent, style=wx.BORDER_RAISED)

        textName = wx.StaticText(self, label='Channel')
        self._textName = wx.StaticText(self,
                                       style=wx.ALIGN_RIGHT |
                                       wx.ST_NO_AUTORESIZE)
        textTimebase = wx.StaticText(self, label='Timebase')
        self._textTimebase = wx.StaticText(self,
                                           style=wx.ALIGN_RIGHT |
                                           wx.ST_NO_AUTORESIZE)
        textSens = wx.StaticText(self, label='Sensitivity')
        self._textSens = wx.StaticText(self,
                                       style=wx.ALIGN_RIGHT |
                                       wx.ST_NO_AUTORESIZE)
        textOffset = wx.StaticText(self, label='Offset')
        self._textOffset = wx.StaticText(self,
                                         style=wx.ALIGN_RIGHT |
                                         wx.ST_NO_AUTORESIZE)
        textAtten = wx.StaticText(self, label='Attenuation')
        self._textAtten = wx.StaticText(self,
                                        style=wx.ALIGN_RIGHT |
                                        wx.ST_NO_AUTORESIZE)

        sizer = wx.GridBagSizer(vgap=4, hgap=4)
        sizer.Add(textName, pos=(0, 0))
        sizer.Add(self._textName, pos=(0, 1), flag=wx.EXPAND)
        sizer.Add(textTimebase, pos=(1, 0))
        sizer.Add(self._textTimebase, pos=(1, 1), flag=wx.EXPAND)
        sizer.Add(textSens, pos=(2, 0))
        sizer.Add(self._textSens, pos=(2, 1), flag=wx.EXPAND)
        sizer.Add(textOffset, pos=(3, 0))
        sizer.Add(self._textOffset, pos=(3, 1), flag=wx.EXPAND)
        sizer.Add(textAtten, pos=(4, 0))
        sizer.Add(self._textAtten, pos=(4, 1), flag=wx.EXPAND)
        sizer.AddGrowableCol(1, 1)

        self.update()

        self.SetSizerAndFit(sizer)

    def update(self, channel=None):
        name = ''
        timebase = ''
        sensitivity = ''
        offset = ''
        attenuation = ''

        if channel is not None:
            name = '{}'.format(channel.name)
            timebase = '{}s'.format(format_eng(channel.timebase))
            sensitivity = '{}v'.format(format_eng(channel.sensitivity))
            offset = '{}v'.format(format_eng(channel.offset))
            attenuation = '{}X'.format(channel.attenuation)

        self._textName.SetLabel(name)
        self._textTimebase.SetLabel(timebase)
        self._textSens.SetLabel(sensitivity)
        self._textOffset.SetLabel(offset)
        self._textAtten.SetLabel(attenuation)
        self.Layout()


def format_eng(value):
    if value == 0:
        return 0

    suffixes = 'num_kM'

    exp = int(math.floor(math.log10(abs(value))))
    engExp = exp - (exp % 3)
    valueEng = value / (10 ** engExp)

    if engExp >= -9 and engExp <= 6 and engExp != 0:
        suffix = suffixes[(engExp + 9) / 3]
    else:
        suffix = ''

    return '{}{}'.format(valueEng, suffix)

if __name__ == '__main__':
    app = wx.App(False)
    frame = FrameOscilloscope()
    frame.Show()
    app.MainLoop()
