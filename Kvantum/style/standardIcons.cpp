/*
 * Copyright (C) Pedram Pourang (aka Tsu Jan) 2019-2024 <tsujan2000@gmail.com>
 *
 * Kvantum is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Kvantum is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Kvantum.h"

#include <QApplication>
#include <QPainter>
#include <QWindow>
#include <QLineEdit>
#include <QComboBox>
#include <QDockWidget>
#include <QSvgRenderer>
#include <QMdiSubWindow>

namespace Kvantum
{

QIcon Style::standardIcon(QStyle::StandardPixmap standardIcon,
                          const QStyleOption *option,
                          const QWidget *widget) const
{
  QWindow *win = widget ? widget->window()->windowHandle() : nullptr;
  qreal pixelRatio = win ? win->devicePixelRatio() : qApp->devicePixelRatio();
  pixelRatio = qMax(pixelRatio, static_cast<qreal>(1));
  const bool rtl(option != nullptr ? option->direction == Qt::RightToLeft
                                   : QApplication::layoutDirection() == Qt::RightToLeft);
  switch (standardIcon) {
    case SP_ToolBarHorizontalExtensionButton : {
      indicator_spec dspec = getIndicatorSpec(KSL("IndicatorArrow"));
      int s = qRound(pixelRatio*pixelRatio*static_cast<qreal>(dspec.size));
      QPixmap pm(QSize(s,s));
      pm.fill(Qt::transparent);

      QPainter painter(&pm);

      /* dark-and-light themes */
      if (themeRndr_ && themeRndr_->isValid())
      {
        /* for toolbar, widget is null but option isn't (Qt -> qtoolbarextension.cpp);
           for menubar, widget isn't null but option is (Qt -> qmenubar.cpp) */
        QColor col;
        if (!widget // unfortunately, there's no way to tell if it's a stylable toolbar :(
            || isStylableToolbar(widget) // doesn't happen
            || mergedToolbarHeight(widget) > 0)
        {
          col = getFromRGBA(getLabelSpec(KSL("Toolbar")).normalColor);
        }
        else if (widget)
          col = getFromRGBA(getLabelSpec(KSL("MenuBar")).normalColor);
        if (enoughContrast(col, standardPalette().color(QPalette::Active,QPalette::WindowText))
            && themeRndr_->elementExists("flat-"+dspec.element+"-down-normal"))
        {
          dspec.element = "flat-"+dspec.element;
        }
      }

      if (renderElement(&painter,
                        dspec.element
                          + (rtl ? "-left" : "-right")
                          + "-normal",
                        QRect(0,0,s,s)))
      {
        return QIcon(pm);
      }
      else break;
    }
    case SP_ToolBarVerticalExtensionButton : {
      indicator_spec dspec = getIndicatorSpec(KSL("IndicatorArrow"));
      int s = qRound(pixelRatio*pixelRatio*static_cast<qreal>(dspec.size));
      QPixmap pm(QSize(s,s));
      pm.fill(Qt::transparent);

      QPainter painter(&pm);

      if (hspec_.style_vertical_toolbars
          && themeRndr_ && themeRndr_->isValid()
          && enoughContrast(getFromRGBA(getLabelSpec(KSL("Toolbar")).normalColor),
                            standardPalette().color(QPalette::Active,QPalette::WindowText)))
      {
        dspec.element = "flat-"+dspec.element;
      }

      if (renderElement(&painter, dspec.element+"-down-normal", QRect(0,0,s,s)))
        return QIcon(pm);
      else break;
    }
    case SP_LineEditClearButton : {
      QIcon icn;
      QString str = (option ? rtl : widget ? widget->layoutDirection() == Qt::RightToLeft : rtl)
                      ? "edit-clear-locationbar-ltr" : "edit-clear-locationbar-rtl";
      if (QIcon::hasThemeIcon(str))
        icn = QIcon::fromTheme(str);
      else
      {
        str = "edit-clear";
        if (QIcon::hasThemeIcon(str))
          icn = QIcon::fromTheme(str);
      }
      if (!icn.isNull())
      {
        if (option || widget)
        { // also correct the color of the symbolic clear icon  (-> CE_ToolBar)
          if (enoughContrast(standardPalette().color(QPalette::Active,QPalette::Text),
                             option ? option->palette.color(QPalette::Active,QPalette::Text)
                                    : widget->palette().color(QPalette::Active,QPalette::Text)))
          {
            const int s = pixelMetric(PM_SmallIconSize);
            QPixmap px = getPixmapFromIcon(icn,
                                           (option ? (option->state & State_Enabled) : widget->isEnabled())
                                             ? Selected : DisabledSelected,
                                           QIcon::On, QSize(s,s));
            icn = QIcon(px);
          }
        }
        return icn;
      }
      else break;
    }
    case SP_TitleBarMinButton : {
      int s = qRound(pixelMetric(PM_TitleBarButtonIconSize, option, widget)*pixelRatio*pixelRatio);
      QPixmap pm(QSize(s,s));
      pm.fill(Qt::transparent);

      /* no menu icon without enough contrast (see Qt ->
         qmdisubwindow.cpp -> QMdiSubWindowPrivate::createSystemMenu) */
      if (option == nullptr && qobject_cast<const QMdiSubWindow*>(widget)
          && enoughContrast(getFromRGBA(getLabelSpec(KSL("MenuItem")).normalColor),
                            getFromRGBA(getLabelSpec(KSL("TitleBar")).focusColor)))
       return QIcon(pm);

      QPainter painter(&pm);

      QString status("normal");
      if (option)
        status = (option->state & State_Enabled) ?
                   (option->state & State_Sunken) ? "pressed" :
                   (option->state & State_MouseOver) ? "focused" : "normal"
                 : "disabled";
      if (renderElement(&painter,
                        getIndicatorSpec(KSL("TitleBar")).element+"-minimize-"+status,
                        QRect(0,0,s,s)))
        return QIcon(pm);
      else break;
    }
    case SP_TitleBarMaxButton : {
      int s = qRound(pixelMetric(PM_TitleBarButtonIconSize, option, widget)*pixelRatio*pixelRatio);
      QPixmap pm(QSize(s,s));
      pm.fill(Qt::transparent);

      if (option == nullptr && qobject_cast<const QMdiSubWindow*>(widget)
          && enoughContrast(getFromRGBA(getLabelSpec(KSL("MenuItem")).normalColor),
                            getFromRGBA(getLabelSpec(KSL("TitleBar")).focusColor)))
       return QIcon(pm); // no menu icon without enough contrast

      QPainter painter(&pm);

      if (renderElement(&painter,getIndicatorSpec(KSL("TitleBar")).element+"-maximize-normal",QRect(0,0,s,s)))
        return QIcon(pm);
      else break;
    }
    case SP_DockWidgetCloseButton :
    case SP_TitleBarCloseButton : {
      int s = qRound(pixelMetric(PM_TitleBarButtonIconSize, option, widget)*pixelRatio*pixelRatio);
      QPixmap pm(QSize(s,s));
      pm.fill(Qt::transparent);

      if (standardIcon == SP_TitleBarCloseButton
          && option == nullptr && qobject_cast<const QMdiSubWindow*>(widget)
          && enoughContrast(getFromRGBA(getLabelSpec(KSL("MenuItem")).normalColor),
                            getFromRGBA(getLabelSpec(KSL("TitleBar")).focusColor)))
       return QIcon(pm); // no menu icon without enough contrast

      QPainter painter(&pm);

      QString status("normal");
      if (qstyleoption_cast<const QStyleOptionButton*>(option))
      {
        status = (option->state & State_Enabled) ?
                   (option->state & State_Sunken) ? "pressed" :
                   (option->state & State_MouseOver) ? "focused" : "normal"
                 : "disabled";
      }
      bool rendered(false);
      if (standardIcon == SP_DockWidgetCloseButton
          || qobject_cast<const QDockWidget*>(widget))
      {
        rendered = renderElement(&painter,
                                 getIndicatorSpec(KSL("Dock")).element+"-close",
                                 QRect(0,0,s,s));
      }
      if (!rendered)
        rendered = renderElement(&painter,
                                 getIndicatorSpec(KSL("TitleBar")).element+"-close-"+status,
                                 QRect(0,0,s,s));
      if (rendered)
        return QIcon(pm);
      else break;
    }
    case SP_TitleBarMenuButton : {
      int s = qRound(pixelMetric(PM_TitleBarButtonIconSize, option, widget)*pixelRatio*pixelRatio);
      QPixmap pm(QSize(s,s));
      pm.fill(Qt::transparent);

      QPainter painter(&pm);

      if (renderElement(&painter,getIndicatorSpec(KSL("TitleBar")).element+"-menu-normal",QRect(0,0,s,s)))
        return QIcon(pm);
      else break;
    }
    case SP_TitleBarNormalButton : {
      int s = qRound(pixelMetric(PM_TitleBarButtonIconSize, option, widget)*pixelRatio*pixelRatio);
      QPixmap pm(QSize(s,s));
      pm.fill(Qt::transparent);

      if (option == nullptr && qobject_cast<const QMdiSubWindow*>(widget)
          && enoughContrast(getFromRGBA(getLabelSpec(KSL("MenuItem")).normalColor),
                            getFromRGBA(getLabelSpec(KSL("TitleBar")).focusColor)))
       return QIcon(pm); // no menu icon without enough contrast

      QPainter painter(&pm);

      QString status("normal");
      if (qstyleoption_cast<const QStyleOptionButton*>(option))
      {
        status = (option->state & State_Enabled) ?
                   (option->state & State_Sunken) ? "pressed" :
                   (option->state & State_MouseOver) ? "focused" : "normal"
                 : "disabled";
      }
      bool rendered(false);
      if (qobject_cast<const QDockWidget*>(widget))
        rendered = renderElement(&painter,
                                 getIndicatorSpec(KSL("Dock")).element+"-restore",
                                 QRect(0,0,s,s));
      if (!rendered)
        rendered = renderElement(&painter,
                                 getIndicatorSpec(KSL("TitleBar")).element+"-restore-"+status,
                                 QRect(0,0,s,s));
      if (rendered)
        return QIcon(pm);
      else break;
    }

    /* file system icons */
    case SP_DriveFDIcon : {
      QIcon icn = QIcon::fromTheme(KSL("media-floppy"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DriveHDIcon : {
      QIcon icn = QIcon::fromTheme(KSL("drive-harddisk"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DriveCDIcon :
    case SP_DriveDVDIcon : {
      QIcon icn = QIcon::fromTheme(KSL("media-optical"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_TrashIcon : {
      QIcon icn = QIcon::fromTheme(KSL("user-trash"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DesktopIcon : {
      QIcon icn = QIcon::fromTheme(KSL("user-desktop"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_ComputerIcon : {
      QIcon icn = QIcon::fromTheme(KSL("computer"),
                                   QIcon::fromTheme(KSL("system")));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DirClosedIcon :
    case SP_DirIcon : {
      QIcon icn = QIcon::fromTheme(KSL("folder"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DirOpenIcon : {
      QIcon icn = QIcon::fromTheme(KSL("folder-open"));
      if (!icn.isNull()) return icn;
      else break;
    }

    /* arrow icons */
    case SP_ArrowUp : {
      QIcon icn = QIcon::fromTheme(KSL("go-up"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_ArrowDown : {
      QIcon icn = QIcon::fromTheme(KSL("go-down"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_ArrowRight : {
      QIcon icn = QIcon::fromTheme(KSL("go-next"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_ArrowLeft : {
      QIcon icn = QIcon::fromTheme(KSL("go-previous"));
      if (!icn.isNull()) return icn;
      else break;
    }

    /* process icons */
    case SP_BrowserReload : {
      QIcon icn = QIcon::fromTheme(KSL("view-refresh"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_BrowserStop : {
      QIcon icn = QIcon::fromTheme(KSL("process-stop"));
      if (!icn.isNull()) return icn;
      else break;
    }

    /* media icons */
    case SP_MediaPlay : {
      QIcon icn = QIcon::fromTheme(KSL("media-playback-start"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_MediaPause : {
      QIcon icn = QIcon::fromTheme(KSL("media-playback-pause"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_MediaStop : {
      QIcon icn = QIcon::fromTheme(KSL("media-playback-stop"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_MediaSeekForward : {
      QIcon icn = QIcon::fromTheme(KSL("media-seek-forward"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_MediaSeekBackward : {
      QIcon icn = QIcon::fromTheme(KSL("media-seek-backward"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_MediaSkipForward : {
      QIcon icn = QIcon::fromTheme(KSL("media-skip-forward"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_MediaSkipBackward : {
      QIcon icn = QIcon::fromTheme(KSL("media-skip-backward"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_MediaVolume : {
      QIcon icn = QIcon::fromTheme(KSL("audio-volume-medium"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_MediaVolumeMuted : {
      QIcon icn = QIcon::fromTheme(KSL("audio-volume-muted"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_CommandLink :
    case SP_ArrowForward : {
      if (rtl)
        return QCommonStyle::standardIcon(SP_ArrowLeft, option, widget);
      return QCommonStyle::standardIcon(SP_ArrowRight, option, widget);
    }
    case SP_ArrowBack : {
      if (rtl)
        return QCommonStyle::standardIcon(SP_ArrowRight, option, widget);
      return QCommonStyle::standardIcon(SP_ArrowLeft, option, widget);
    }

    /* link icons */
    case SP_FileLinkIcon : {;
      QIcon icn = QIcon::fromTheme(KSL("emblem-symbolic-link"));
      if (!icn.isNull())
      {
        QIcon baseIcon = QCommonStyle::standardIcon(SP_FileIcon, option, widget);
        const QList<QSize> sizes = baseIcon.availableSizes(QIcon::Normal, QIcon::Off);
        for (int i = 0 ; i < sizes.size() ; ++i)
        {
          int size = sizes[i].width();
          QPixmap basePixmap = baseIcon.pixmap(QSize(size, size), pixelRatio);
          QPixmap linkPixmap = icn.pixmap(QSize(size / 2, size / 2), pixelRatio);
          QPainter painter(&basePixmap);
          painter.drawPixmap(size/2, size/2, linkPixmap);
          icn.addPixmap(basePixmap);
        }
        return icn;
      }
      break;
    }
    case SP_DirLinkIcon : {;
      QIcon icn = QIcon::fromTheme(KSL("emblem-symbolic-link"));
      if (!icn.isNull())
      {
        QIcon baseIcon = QCommonStyle::standardIcon(SP_DirIcon, option, widget);
        const QList<QSize> sizes = baseIcon.availableSizes(QIcon::Normal, QIcon::Off);
        for (int i = 0 ; i < sizes.size() ; ++i)
        {
          int size = sizes[i].width();
          QPixmap basePixmap = baseIcon.pixmap(QSize(size, size), pixelRatio);
          QPixmap linkPixmap = icn.pixmap(QSize(size / 2, size / 2), pixelRatio);
          QPainter painter(&basePixmap);
          painter.drawPixmap(size/2, size/2, linkPixmap);
          icn.addPixmap(basePixmap);
        }
        return icn;
      }
      break;
    }

    /* dialog icons */
    case SP_DialogCloseButton : {
      QIcon icn = QIcon::fromTheme(KSL("dialog-close"),
                                   QIcon::fromTheme(KSL("window-close")));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogOpenButton : {
      QIcon icn = QIcon::fromTheme(KSL("document-open"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogApplyButton : {
      QIcon icn = QIcon::fromTheme(KSL("dialog-ok-apply"),
                                   QIcon::fromTheme(KSL("dialog-ok")));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogYesToAllButton :
    case SP_DialogYesButton :
    case SP_DialogOkButton : {
      QIcon icn = QIcon::fromTheme(KSL("dialog-ok"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogNoToAllButton :
    case SP_DialogAbortButton :
    case SP_DialogIgnoreButton :
    case SP_DialogCancelButton :
    case SP_DialogNoButton : {
      QIcon icn = QIcon::fromTheme(KSL("dialog-cancel"),
                                   QIcon::fromTheme(KSL("process-stop")));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogSaveButton : {
      QIcon icn = QIcon::fromTheme(KSL("document-save"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogResetButton : {
      QIcon icn = QIcon::fromTheme(KSL("edit-clear"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogHelpButton : {
      QIcon icn = QIcon::fromTheme(KSL("help-contents"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_FileDialogDetailedView : {
      QIcon icn = QIcon::fromTheme(KSL("view-list-details"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_FileDialogToParent : {
      QIcon icn = QIcon::fromTheme(KSL("go-up"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_FileDialogNewFolder : {
      QIcon icn = QIcon::fromTheme(KSL("folder-new"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogSaveAllButton : {
      QIcon icn = QIcon::fromTheme(KSL("document-save-all"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_DialogRetryButton : {
      QIcon icn = QIcon::fromTheme(KSL("view-refresh"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_RestoreDefaultsButton : {
      QIcon icn = QIcon::fromTheme(KSL("edit-undo"));
      if (!icn.isNull()) return icn;
      else break;
    }
    // these are for LXQt file dialog
    case SP_FileDialogListView : {
      QIcon icn = QIcon::fromTheme(KSL("view-list-text"));
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_FileDialogInfoView : {
      QIcon icn = QIcon::fromTheme(KSL("dialog-information")); // document-properties
      if (!icn.isNull()) return icn;
      else break;
    }
    case SP_FileDialogContentsView : {
      QIcon icn = QIcon::fromTheme(KSL("view-list-icons"));
      if (!icn.isNull()) return icn;
      else break;
    }

    case SP_TabCloseButton : {
      QIcon icn = QIcon::fromTheme(KSL("tab-close"),
                                   QIcon::fromTheme(KSL("window-close")));
      if (!icn.isNull()) return icn;
      else break;
    }

    default : break;
  }

  return QCommonStyle::standardIcon(standardIcon,option,widget);
}

}
