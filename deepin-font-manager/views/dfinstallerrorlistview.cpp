#include "dfinstallerrorlistview.h"
#include "globaldef.h"

#include <QPainter>
#include <QMouseEvent>

#include <DLog>
#include <DStyleHelper>
#include <DApplication>
#include <DApplicationHelper>

//DFInstallErrorListDelegate
DFInstallErrorListDelegate::DFInstallErrorListDelegate(QAbstractItemView *parent)
    :DStyledItemDelegate(parent)
    , m_parentView(parent)
{
}

//用于去除选中项的边框
void DFInstallErrorListDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option,
                             const QModelIndex &index) const
{
    if (index.isValid()) {

        painter->save();
        painter->setRenderHint(QPainter::Antialiasing, true);

        QVariant varErrorItem = index.data(Qt::DisplayRole);
        DFInstallErrorItemModel itemModel = varErrorItem.value<DFInstallErrorItemModel>();

        QString strFontFileName = itemModel.strFontFileName;
        QString strStatus = itemModel.strFontInstallStatus;

        QStyleOptionViewItem viewOption(option);  //用来在视图中画一个item

        DPalette::ColorGroup cg = option.state & QStyle::State_Enabled
                                  ? DPalette::Normal : DPalette::Disabled;
        if (cg == DPalette::Normal && !(option.state & QStyle::State_Active)) {
            cg = DPalette::Inactive;
        }

        QRect rect;
        rect.setX(option.rect.x());
        rect.setY(option.rect.y());
        rect.setWidth(option.rect.width());
        rect.setHeight(option.rect.height());

        QRect bgRect = QRect(rect.left()+10, rect.top(), rect.width()-20, rect.height());

        if (itemModel.bSelectable) {

            QPainterPath path;
            const int radius = 8;

            path.moveTo(bgRect.bottomRight() - QPoint(0, radius));
            path.lineTo(bgRect.topRight() + QPoint(0, radius));
            path.arcTo(QRect(QPoint(bgRect.topRight() - QPoint(radius * 2, 0)), QSize(radius * 2, radius * 2)), 0, 90);
            path.lineTo(bgRect.topLeft() + QPoint(radius, 0));
            path.arcTo(QRect(QPoint(bgRect.topLeft()), QSize(radius * 2, radius * 2)), 90, 90);
            path.lineTo(bgRect.bottomLeft() - QPoint(0, radius));
            path.arcTo(QRect(QPoint(bgRect.bottomLeft() - QPoint(0, radius * 2)), QSize(radius * 2, radius * 2)), 180, 90);
            path.lineTo(bgRect.bottomLeft() + QPoint(radius, 0));
            path.arcTo(QRect(QPoint(bgRect.bottomRight() - QPoint(radius * 2, radius * 2)), QSize(radius * 2, radius * 2)), 270, 90);

            if (option.state & QStyle::State_Selected) {
                DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
                DStyleHelper styleHelper;
                QColor fillColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::ItemBackground);
                painter->fillPath(path, QBrush(fillColor));
            }
        }

        //绘制复选框
        int checkBoxSize = 20;
        //绘制checkbox
        QStyleOptionButton checkBoxOption;
        bool checked = itemModel.bChecked;
        checkBoxOption.state |= QStyle::State_Enabled;
        //根据值判断是否选中
        if (checked) {
            checkBoxOption.state |= QStyle::State_On;
        } else {
            checkBoxOption.state |= QStyle::State_Off;
        }

        DCheckBox checkBox;
        QRect checkboxRect = QRect(bgRect.left() + 8, bgRect.top()+14+2, checkBoxSize-4, checkBoxSize-4);
        checkBoxOption.rect = checkboxRect;
        DApplication::style()->drawPrimitive(QStyle::PE_IndicatorCheckBox,
                                             &checkBoxOption,
                                             painter,
                                             &checkBox);

        int statusLabelMaxWidth = 80;
        QRect fontFileNameRect = QRect(bgRect.left()+39, bgRect.top(), bgRect.width()-39-statusLabelMaxWidth, bgRect.height());

        QFont nameFont;
        nameFont.setPixelSize(14);
        painter->setFont(nameFont);

        if (option.state & QStyle::State_Selected) {
            painter->setPen(QPen(option.palette.color(DPalette::Text)));
            painter->drawText(fontFileNameRect, Qt::AlignLeft | Qt::AlignVCenter, strFontFileName);
        }
        else {
            painter->setPen(QPen(option.palette.color(DPalette::Text)));
            painter->drawText(fontFileNameRect, Qt::AlignLeft | Qt::AlignVCenter, strFontFileName);
        }

        QRect installStatusRect = QRect(bgRect.left()+(bgRect.width()-statusLabelMaxWidth)-10,
                                        bgRect.top(),
                                        statusLabelMaxWidth,
                                        bgRect.height());

        QFont installStatusFont;
        installStatusFont.setPixelSize(11);
        painter->setFont(installStatusFont);

        DPalette pa = DApplicationHelper::instance()->palette(m_parentView);
        DStyleHelper styleHelper;
        QColor penColor = styleHelper.getColor(static_cast<const QStyleOption *>(&option), pa, DPalette::TextWarning);
        painter->setPen(QPen(penColor));
        painter->drawText(installStatusRect, Qt::AlignRight | Qt::AlignVCenter, strStatus);

        painter->restore();
    } else {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize DFInstallErrorListDelegate::sizeHint(const QStyleOptionViewItem &option,
                                         const QModelIndex &index) const
{
    Q_UNUSED(index)

    return QSize(option.rect.width(), 48);
}


//DFInstallErrorListView
DFInstallErrorListView::DFInstallErrorListView(QList<DFInstallErrorItemModel> installErrorFontModelList,
                                               QWidget *parent)
    : DListView(parent)
    , m_installErrorFontModelList(installErrorFontModelList)
{
    setAutoScroll(true);
    setMouseTracking(true);

    initErrorListData();
    initDelegate();
}

DFInstallErrorListView::~DFInstallErrorListView()
{
}

void DFInstallErrorListView::initErrorListData()
{
    m_errorListSourceModel = new QStandardItemModel;

    for (int i = 0; i < m_installErrorFontModelList.size(); i++) {

        QStandardItem *item = new QStandardItem;
        DFInstallErrorItemModel itemModel = m_installErrorFontModelList.at(i);
        item->setData(QVariant::fromValue(itemModel), Qt::DisplayRole);

        m_errorListSourceModel->appendRow(item);
    }

    this->setModel(m_errorListSourceModel);

    //设置默认选中第一行
    QModelIndex firstRowModelIndex = m_errorListSourceModel->index(0, 0);

    DFInstallErrorItemModel itemModel =
        qvariant_cast<DFInstallErrorItemModel>(m_errorListSourceModel->data(firstRowModelIndex));

    if (itemModel.bSelectable) {
        selectionModel()->select(firstRowModelIndex, QItemSelectionModel::Select);
    }
}

void DFInstallErrorListView::initDelegate()
{
    m_errorListItemDelegate = new DFInstallErrorListDelegate(this);
    this->setItemDelegate(m_errorListItemDelegate);
}

QStandardItemModel* DFInstallErrorListView::getErrorListSourceModel()
{
    return m_errorListSourceModel;
}

void DFInstallErrorListView::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_bLeftMouse = true;
    } else {
        m_bLeftMouse = false;
    }

    DListView::mousePressEvent(event);
}

void DFInstallErrorListView::setSelection(const QRect &rect, QItemSelectionModel::SelectionFlags command)
{
    DListView::setSelection(rect, command);

    QPoint selectionPoint(rect.x(), rect.y());
    QModelIndex modelIndex = indexAt(selectionPoint);

    if (m_bLeftMouse) {

        DFInstallErrorItemModel itemModel =
            qvariant_cast<DFInstallErrorItemModel>(m_errorListSourceModel->data(modelIndex));
        if (!itemModel.bSelectable) {
            return;
        }

        if (selectionPoint.x() < 39) {

            emit onClickErrorListItem(modelIndex);
        }
    }
}
