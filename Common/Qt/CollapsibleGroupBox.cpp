/*  Collapsible Group Box
 *
 *  From: https://github.com/PokemonAutomation/
 *
 */

#include <QVBoxLayout>
#include <QAbstractButton>
#include <QKeyEvent>
#include <QPainter>
#include <QStyle>
#include <QStyleOption>
#include "CollapsibleGroupBox.h"

namespace PokemonAutomation{

namespace{

constexpr int HEADER_PADDING = 8;
constexpr int ARROW_SIZE = 12;
constexpr int ARROW_SPACING = 6;
constexpr int CONTENT_INDENT = HEADER_PADDING + ARROW_SIZE + ARROW_SPACING;

class SectionHeader : public QAbstractButton{
public:
    SectionHeader(QWidget* parent, const QString& title, Qt::Orientation orientation)
        : QAbstractButton(parent)
        , m_orientation(orientation)
    {
        setText(title);
        setCheckable(true);
        setFocusPolicy(Qt::StrongFocus);
        setAttribute(Qt::WA_Hover);
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
        QFont header_font = font();
        header_font.setBold(true);
        setFont(header_font);
    }

    QSize sizeHint() const override{
        QSize size(
            CONTENT_INDENT + fontMetrics().horizontalAdvance(text()) + HEADER_PADDING,
            qMax(ARROW_SIZE, fontMetrics().height()) + 2 * HEADER_PADDING
        );
        if (compact()){
            size.transpose();
        }
        return size;
    }
    QSize minimumSizeHint() const override{
        if (compact()){
            return sizeHint();
        }
        return QSize(CONTENT_INDENT + HEADER_PADDING, sizeHint().height());
    }

private:
    bool compact() const{
        return m_orientation == Qt::Horizontal && !isChecked();
    }

    const Qt::Orientation m_orientation;

protected:
    bool event(QEvent* event) override{
        if (event->type() == QEvent::HoverEnter || event->type() == QEvent::HoverLeave){
            update();
        }
        return QAbstractButton::event(event);
    }

    void paintEvent(QPaintEvent*) override{
        QStyleOption option;
        option.initFrom(this);
        QPainter painter(this);

        painter.fillRect(rect(), option.palette.brush(QPalette::Button));
        //  Some themes use the same color for Button and Window.
        QColor shade = option.palette.color(QPalette::Mid);
        shade.setAlpha(24);
        painter.fillRect(rect(), shade);
        if (isEnabled() && (underMouse() || isDown())){
            QColor hover = option.palette.color(QPalette::Highlight);
            hover.setAlpha(isDown() ? 55 : 25);
            painter.fillRect(rect(), hover);
        }

        painter.setPen(option.palette.color(QPalette::Mid));
        painter.drawLine(0, height() - 1, width() - 1, height() - 1);
        if (hasFocus()){
            painter.setPen(option.palette.color(QPalette::Highlight));
            painter.drawRect(rect().adjusted(0, 0, -1, -1));
        }

        option.rect = QStyle::visualRect(layoutDirection(), rect(), QRect(
            compact() ? (width() - ARROW_SIZE) / 2 : HEADER_PADDING,
            compact() ? HEADER_PADDING : (height() - ARROW_SIZE) / 2,
            ARROW_SIZE, ARROW_SIZE
        ));
        QStyle::PrimitiveElement arrow = QStyle::PE_IndicatorArrowRight;
        if (isChecked()){
            arrow = m_orientation == Qt::Horizontal
                ? QStyle::PE_IndicatorArrowLeft : QStyle::PE_IndicatorArrowDown;
        }
        style()->drawPrimitive(
            arrow, &option, &painter, this
        );

        QRect text_rect = QStyle::visualRect(layoutDirection(), rect(), rect().adjusted(
            CONTENT_INDENT, 0, -HEADER_PADDING, 0
        ));
        if (compact()){
            //  Read the collapsed title from top to bottom beneath the arrow.
            painter.translate(width(), CONTENT_INDENT);
            painter.rotate(90);
            text_rect = QRect(0, 0, height() - CONTENT_INDENT - HEADER_PADDING, width());
        }
        style()->drawItemText(
            &painter, text_rect, Qt::AlignVCenter | Qt::AlignLeading | Qt::TextShowMnemonic,
            option.palette, isEnabled(),
            fontMetrics().elidedText(text(), Qt::ElideRight, qMax(0, text_rect.width())),
            QPalette::ButtonText
        );
    }
};

}


CollapsibleGroupBox::CollapsibleGroupBox(
    QWidget& parent, const QString& title, bool expanded, Qt::Orientation orientation
)
    : QWidget(&parent)
    , m_header(new SectionHeader(this, title, orientation))
    , m_content(new QWidget(this))
    , m_widget(nullptr)
    , m_orientation(orientation)
{
    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    m_header->setChecked(expanded);
    layout->addWidget(m_header);
    layout->addWidget(m_content, orientation == Qt::Horizontal ? 1 : 0);
    if (orientation == Qt::Horizontal){
        //  Keep the reopen button at the top of the collapsed sidebar.
        layout->addStretch(0);
    }

    QVBoxLayout* content_layout = new QVBoxLayout(m_content);
    if (orientation == Qt::Horizontal){
        content_layout->setContentsMargins(0, HEADER_PADDING, 0, 0);
    }else{
        content_layout->setContentsMargins(HEADER_PADDING, HEADER_PADDING, HEADER_PADDING, HEADER_PADDING);
    }
    content_layout->setSpacing(0);

    set_expanded(expanded);

    connect(
        m_header, &QAbstractButton::toggled,
        this, [this](bool on){
            set_expanded(on);
        }
    );
}
void CollapsibleGroupBox::set_expanded(bool expanded){
    m_content->setVisible(expanded && m_widget != nullptr);
    if (m_orientation == Qt::Horizontal){
        static_cast<QVBoxLayout*>(layout())->setStretch(2, expanded ? 0 : 1);
        m_header->setToolTip((expanded ? tr("Collapse %1") : tr("Expand %1")).arg(m_header->text()));
        m_header->updateGeometry();
        if (expanded){
            setMinimumWidth(0);
            setMaximumWidth(QWIDGETSIZE_MAX);
        }else{
            setFixedWidth(m_header->sizeHint().width());
        }
        updateGeometry();
    }
}

QWidget* CollapsibleGroupBox::widget(){
    return m_widget;
}
void CollapsibleGroupBox::set_widget(QWidget* widget){
    if (widget == m_widget){
        return;
    }

    delete m_widget;
    m_widget = widget;
    if (widget != nullptr){
        widget->setParent(m_content);
        m_content->layout()->addWidget(widget);
        widget->show();
    }
    set_expanded(m_header->isChecked());
}

}
