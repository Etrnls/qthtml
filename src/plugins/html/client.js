/****************************************************************************
**
** Copyright (C) 2011 by Etrnls
** etrnls@gmail.com
**
** This file is part of the Qt HTML service daemon.
**
** Qt HTML service daemon is free software: you can redistribute it and/or
** modify it under the terms of the GNU General Public License as
** published by the Free Software Foundation, either version 3 of the License,
** or (at your option) any later version.
**
** Qt HTML service daemon is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with Qt HTML service daemon. If not, see
** <http://www.gnu.org/licenses/>.
**
****************************************************************************/

// disable dialog containment
(function($) {
    $.ui.dialog.prototype._makeDraggableBase = $.ui.dialog.prototype._makeDraggable;
    $.ui.dialog.prototype._makeDraggable = function() {
        this._makeDraggableBase();
        this.uiDialog.draggable('option', 'containment', false);
    };
})(jQuery);

// mousewheel event
(function($) {

var mousewheelEvents = ['DOMMouseScroll', 'mousewheel'];

$.each(mousewheelEvents, function(i, name) {
    $.event.fixHooks[name] = $.event.mouseHooks;
});

function mousewheelHandler() {
    return $.event.dispatch.apply(this, arguments);
}

$.event.special.mousewheel = {
    setup : function() {
        if (this.addEventListener) {
            var obj = this;
            $.each(mousewheelEvents, function(i, name) {
                obj.addEventListener(name, mousewheelHandler, false);
            });
        } else if (this.attachEvent) {
            this.attachEvent('onmousewheel', mousewheelHandler);
        }
    },

    teardown: function() {
        if (this.removeEventListener) {
            var obj = this;
            $.each(mousewheelEvents, function(i, name) {
                obj.removeEventListener(name, mousewheelHandler, false);
            });
        } else if (this.detachEvent) {
            this.detachEvent('onmousewheel', mousewheelHandler);
        }
    },

    handle: function(event) {
        var originalEvent = event.originalEvent;
        if (originalEvent.wheelDelta)
            event.delta = originalEvent.wheelDelta / 120;
        else if (originalEvent.detail)
            event.delta = -originalEvent.detail / 3;
        event.handleObj.handler.call(this, event);
    }
};

$.fn['mousewheel'] = function(fn) {
    return fn ? this.on('mousewheel', null, null, fn) : this.trigger('mousewheel');
};

})(jQuery);

// base64 functions from http://en.wikibooks.org/wiki/Algorithm_Implementation/Miscellaneous/Base64
var base64chars = 'ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/'.split('');
var base64inv = {};
for (var i = 0; i < base64chars.length; ++i)
{
    base64inv[base64chars[i]] = i;
}

function base64Encode(s)
{
    var r = '';
    var p = ''
    var c = s.length % 3;

    if (c > 0) {
        for (; c < 3; ++c) {
            p += '=';
            s += '\0';
        }
    }

    for (c = 0; c < s.length; c += 3) {
        if (c > 0 && (c / 3 * 4) % 76 == 0) {
          r += '\r\n';
        }
        var n = (s.charCodeAt(c) << 16) + (s.charCodeAt(c + 1) << 8) + s.charCodeAt(c + 2);
        n = [(n >>> 18) & 63, (n >>> 12) & 63, (n >>> 6) & 63, n & 63];
        r += base64chars[n[0]] + base64chars[n[1]] + base64chars[n[2]] + base64chars[n[3]];
    }
    return r.substring(0, r.length - p.length) + p;
}

function base64Decode(s)
{
    s = s.replace(new RegExp('[^' + base64chars.join('') + '=]', 'g'), '');

    var p = (s.charAt(s.length - 1) == '=' ? (s.charAt(s.length - 2) == '=' ? 'AA' : 'A') : ''); 
    var r = '';
    s = s.substr(0, s.length - p.length) + p;

    for (var c = 0; c < s.length; c += 4) {
        var n = (base64inv[s.charAt(c)] << 18) + (base64inv[s.charAt(c + 1)] << 12) +
                (base64inv[s.charAt(c + 2)] << 6) + base64inv[s.charAt(c + 3)];
        r += String.fromCharCode((n >>> 16) & 255, (n >>> 8) & 255, n & 255);
    }
    return r.substring(0, r.length - p.length);
}

function utf8Encode(s)
{
    return unescape(encodeURIComponent(s));
}

function utf8Decode(s)
{
    return decodeURIComponent(escape(s));
}

function cancelEvent(event) {
    event.preventDefault();
    event.stopPropagation();
    event.returnValue = false;
    event.cancelBubble = true;
}

var ws = null;
var windows = {};
var mouseButtons = 0;

function translateSpecialKeyCode(code) {
    switch (code) {
    case 8:   return 0x01000003; // Qt::Key_Backspace
    case 9:   return 0x01000001; // Qt::Key_Tab
    case 13:  return 0x01000005; // Qt::Key_Enter
    case 16:  return 0x01000020; // Qt::Key_Shift
    case 17:  return 0x01000021; // Qt::Key_Control
    case 18:  return 0x01000023; // Qt::Key_Alt
    case 27:  return 0x01000000; // Qt::Key_Escape
    case 33:  return 0x01000016; // Qt::Key_PageUp
    case 34:  return 0x01000017; // Qt::Key_PageDown
    case 35:  return 0x01000011; // Qt::Key_End
    case 36:  return 0x01000010; // Qt::Key_Home
    case 37:  return 0x01000012; // Qt::Key_Left
    case 38:  return 0x01000013; // Qt::Key_Up
    case 39:  return 0x01000014; // Qt::Key_Right
    case 40:  return 0x01000015; // Qt::Key_Down
    case 45:  return 0x01000006; // Qt::Key_Insert
    case 46:  return 0x01000007; // Qt::Key_Delete
    case 112: return 0x01000030; // Qt::Key_F1
    case 113: return 0x01000031; // Qt::Key_F2
    case 114: return 0x01000032; // Qt::Key_F3
    case 115: return 0x01000033; // Qt::Key_F4
    case 116: return 0x01000034; // Qt::Key_F5
    case 117: return 0x01000035; // Qt::Key_F6
    case 118: return 0x01000036; // Qt::Key_F7
    case 119: return 0x01000037; // Qt::Key_F8
    case 120: return 0x01000038; // Qt::Key_F8
    case 121: return 0x01000039; // Qt::Key_F10
    case 122: return 0x0100003a; // Qt::Key_F11
    case 123: return 0x0100003b; // Qt::Key_F12
    default: return false;
    }
}

function translateMouseButton(button) {
    switch (button) {
    case 1: return 0x00000001;  // Qt::LeftButton
    case 2: return 0x00000004;  // Qt::MiddleButton
    case 3: return 0x00000002;  // Qt::RightButton
    default: return 0x00000000; // Qt::NoButton
    }
}

function translateModifiers(event) {
    modifiers = 0;
    if (event.shiftKey)     modifiers |= 0x02000000; // Qt::ShiftModifier
    if (event.ctrlKey)      modifiers |= 0x04000000; // Qt::ControlModifier
    if (event.altKey)       modifiers |= 0x08000000; // Qt::AltModifier
    if (event.metaKey)      modifiers |= 0x10000000; // Qt::MetaModifier
    return modifiers;
}

function updateTitleGeometry(win) {
    var dialog = win.parent();
    var padding = $('.ui-dialog-titlebar', dialog).outerWidth() - $('.ui-dialog-titlebar', dialog).width();
    var closeWidth = $('.ui-dialog-titlebar-close', dialog).outerWidth();
    $('.ui-dialog-title', dialog).css('max-width', win.width() - padding - closeWidth);
}

function updateCanvasGeometry(canvas, shadow) {
    $(canvas).width($(canvas).parent().width());
    $(canvas).height($(canvas).parent().height());
    if (shadow) return;

    canvas.width = $(canvas).width();
    canvas.height = $(canvas).height();
}

function isWindowVisible(win) {
    return win.dialog('isOpen') && win.parent().is(':visible');
}

function setWindowGeometry(win, x, y, width, height) {
    if (isWindowVisible(win)) {
        var position = win.dialog('option', 'position');
        var offset = win.offset();
        win.dialog('option', 'position', [x - (offset.left - position[0]), y - (offset.top - position[1])]);
        win.width(width);
        win.height(height);
        updateTitleGeometry(win);
        updateCanvasGeometry(win.canvas);
    } else {
        win.geometry = [x, y, width, height];
    }
}

function getWindowGeometry(win) {
    if (isWindowVisible(win)) {
        var offset = win.offset();
        return [offset.left, offset.top, win.width(), win.height()];
    } else {
        return win.geometry;
    }
}

function getMouseLocalPosition(event) {
    if (event.offsetX && event.offsetY)
        return [event.offsetX, event.offsetY];
    else
        return [Math.round(event.pageX - $(event.target).offset().left),
                Math.round(event.pageY - $(event.target).offset().top)];
}

function getMouseGlobalPosition(event) {
    return [event.pageX, event.pageY];
}

function getMousePosition(event) {
    return $.merge(getMouseLocalPosition(event), getMouseGlobalPosition(event));
}

function getCursorName(shape) {
    switch (shape) {
    case 0: return 'default';
    case 2: return 'crosshair';
    case 3: return 'wait';
    case 4: return 'text';
    case 9: return 'move';
    case 13: return 'pointer';
    case 15: return 'help';
    case 16: return 'wait';
    default: return 'auto';
    }
}

function createWindow(winId, popup) {
    var win = $('<div><canvas /></div>');
    win.canvas = $('canvas', win).get()[0];

    win.mousemove(function(event) {
        sendMessage('~', $.merge($.merge([winId], getMousePosition(event)),
                                 [mouseButtons, translateModifiers(event)]));
    }).mousedown(function(event) {
        mouseButtons |= translateMouseButton(event.which);
        sendMessage('m', $.merge($.merge([winId], getMousePosition(event)),
                                 [mouseButtons, translateModifiers(event)]));
    }).mouseup(function(event) {
        mouseButtons &= ~translateMouseButton(event.which);
        sendMessage('M', $.merge($.merge([winId], getMousePosition(event)),
                                 [mouseButtons, translateModifiers(event)]));
    }).mousewheel(function(event) {
        sendMessage('w', $.merge($.merge([winId], getMousePosition(event)),
                                 [event.delta, translateModifiers(event)]));
        cancelEvent(event);
    }).dialog({
        autoOpen: false,
        position: [0, 0],
        width: 'auto',
        height: 'auto',
        closeOnEscape: false,
        focus: function() {
            sendMessage('a', [winId]);
        },
        beforeClose: function() {
            sendMessage('W', [winId]);
            return false;
        },
        dragStop: function() {
            if (!isWindowVisible(win))  return;
            var position = win.dialog('option', 'position');
            var width = win.width();
            var height = win.height();
            var flag = false;
            if (position[0] + width < 0) {
                position[0] = 0;
                flag = true;
            }
            if (position[0] > $(window).width()) {
                position[0] = $(window).width() - width;
                flag = true;
            }
            if (position[1] < 0) {
                position[1] = 0;
                flag = true;
            }
            if (position[1] > $(window).height()) {
                position[1] = $(window).height() - height;
                flag = true;
            }
            if (flag) {
                win.dialog('option', 'position', position);
            }
            sendMessage('g', $.merge([winId], getWindowGeometry(win)));
        },
        resize: function() {
            updateTitleGeometry(win);
            updateCanvasGeometry(win.canvas, true);
        },
        resizeStop: function() {
            updateTitleGeometry(win);
            updateCanvasGeometry(win.canvas);
            sendMessage('g', $.merge([winId], getWindowGeometry(win)));
        }
    });

    win.parent().keydown(function(event) {
        var code = (event.ctrlKey || event.altKey || event.metaKey)
                   ? event.which
                   : translateSpecialKeyCode(event.which);
        if (code) {
            sendMessage('k', [winId, code, translateModifiers(event), 0]);
            cancelEvent(event);
        }
    }).keyup(function(event) {
        var code = (event.ctrlKey || event.altKey || event.metaKey)
                   ? event.which
                   : translateSpecialKeyCode(event.which);
        if (code) {
            sendMessage('K', [winId, code, translateModifiers(event), 0]);
            cancelEvent(event);
        }
    }).keypress(function(event) {
        sendMessage('k', [winId, event.which, translateModifiers(event), 1]);
        sendMessage('K', [winId, event.which, translateModifiers(event), 1]);
    });

    if (popup) {
        win.dialog('option', 'minWidth', 0);
        win.dialog('option', 'minHeight', 0);
        win.dialog('option', 'dialogClass', 'popup');
        win.dialog('option', 'resizable', false);
    }

    return win;
}

function sendMessage(command, args) {
    //console.log('sendMessage(', command, args, ')');
    args.unshift(command);
    ws.send(args.join(' '));
}

function processMessage(message) {
    var args = message.split(' ');
    var command = args[0];

    switch (command) {
    case 'c': // change cursor
        var shape = parseInt(args[1]);
        var cursor = getCursorName(shape);
        $('body').css('cursor', cursor);
        break;
    case 'w': // create window
        var winId = parseInt(args[1]);
        var popup = parseInt(args[2]) == 1;
        windows[winId] = createWindow(winId, popup);
        break;
    case 'W': // destroy window
        var winId = parseInt(args[1]);
        var win = windows[winId];
        win.dialog('destroy').remove();
        delete windows[winId];
        break;
    case 'g': // set window geometry
        var win = windows[parseInt(args[1])];
        var x = parseInt(args[2]);
        var y = parseInt(args[3]);
        var width = parseInt(args[4]);
        var height = parseInt(args[5]);
        setWindowGeometry(win, x, y, width, height);
        break;
    case 'v': // set visible
        var winId = parseInt(args[1]);
        var win = windows[winId];
        var visible = parseInt(args[2]) == 1;
        if (visible == isWindowVisible(win)) {
            break;
        }
        if (visible) {
            var geometry = win.geometry;
            if (!win.dialog('isOpen')) {
                win.dialog('open');
            }
            win.parent().show();
            setWindowGeometry(win, geometry[0], geometry[1], geometry[2], geometry[3]);
            sendMessage('g', $.merge([winId], getWindowGeometry(win)));
        } else {
            win.geometry = getWindowGeometry(win);
            win.parent().hide();
        }
        break;
    case 't': // set window title
        var win = windows[parseInt(args[1])];
        var title = utf8Decode(base64Decode(args[2]));
        win.dialog('option', 'title', title);
        break;
    case 'r': // raise
        var win = windows[parseInt(args[1])];
        win.dialog('moveToTop');
        break;
    case 'f': // flush
        var win = windows[parseInt(args[1])];
        var x = parseInt(args[2]);
        var y = parseInt(args[3]);
        var width = parseInt(args[4]);
        var height = parseInt(args[5]);
        var image = new Image();
        image.src = args[6];
        image.onload = function() {
            var context = win.canvas.getContext('2d');
            context.globalCompositeOperation = 'source-over';
            context.drawImage(image, x, y);
        };
        break;
    case '+': // scroll
        var win = windows[parseInt(args[1])];
        var x = parseInt(args[2]);
        var y = parseInt(args[3]);
        var width = parseInt(args[4]);
        var height = parseInt(args[5]);
        var dx = parseInt(args[6]);
        var dy = parseInt(args[7]);
        var context = win.canvas.getContext('2d');
        context.globalCompositeOperation = 'copy';
        context.drawImage(context.canvas, x, y, width, height,
                          x + dx, y + dy, width, height);
        break;
    }
}

$(function() {
    var url = 'ws://' + window.location.host + '/socket';

    if ('WebSocket' in window)
        ws = new WebSocket(url, 'qthtml');
    else if ('MozWebSocket' in window)
        ws = new MozWebSocket(url);
    else
        return;

    ws.onopen = function() {
        console.log('ws open');
        $(window).trigger('resize');
    };
    ws.onerror = function() {
        console.log('ws error');
    };
    ws.onclose = function() {
        console.log('ws close');
    };
    ws.onmessage = function(event) {
        processMessage(event.data);
    };

    $(window).resize(function() {
        sendMessage('s', [0, 0, $(window).width(), $(window).height()]);
    });

    $(document).mousedown(function(event) {
        mouseButtons |= translateMouseButton(event.which);
    }).mouseup(function(event) {
        mouseButtons &= ~translateMouseButton(event.which);
    });

    document.oncontextmenu = function() {
        return false;
    };

});
