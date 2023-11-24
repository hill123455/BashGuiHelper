/* 
    BashQtHelp - Use Qt to prompt user for files, list selection, numeric input, date, etcetera
    Programmer: Danny Holstein
 */
#include <QtWidgets/QApplication>
#include <QtWidgets/QFileDialog>
#include <QtWidgets/QListView>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QLabel>
#include <string>
#include <iostream>
#include <map>
#include <stdio.h>
#include "LibXML2.h"

#define ERROR() return 1
#define CANCELLED() return 2

#include <QtGui/QStandardItem>
#include <QtGui/QStandardItemModel>

class SelectionListDialog : public QDialog
{
public:
    SelectionListDialog(QWidget *parent = nullptr, char* items=nullptr) {
        // Create the main layout.
        QVBoxLayout *mainLayout = new QVBoxLayout();

        // Create the tree view.
        QTreeView *treeView = new QTreeView();
        QStandardItemModel *model = new QStandardItemModel();
        QStandardItem *rootItem = model->invisibleRootItem();


        char* token; token = strtok((char*) items, "\t");
        while( token != NULL ) {
            rootItem->appendRow(new QStandardItem(token)); token = strtok(NULL, "\t");}

        treeView->setModel(model);

        // Create the push button.
        QPushButton *pushButton = new QPushButton("OK");

        // Add the widgets to the layout.
        mainLayout->addWidget(treeView);
        mainLayout->addWidget(pushButton);

        // Set the layout for the dialog.
        setLayout(mainLayout);

        // Connect the signals and slots.
        connect(pushButton, &QPushButton::clicked, this, &SelectionListDialog::accept);
    }

    // The slot that will be called when the dialog is accepted.
    void onAccepted()
    {
        // Get the selected items from the tree view.
        QItemSelectionModel *selectionModel = treeView->selectionModel();
        QList<QModelIndex> selectedIndexes = selectionModel->selectedIndexes();

        // Display the selected items in a label.
        // label->setText(qml::join(", ", selectedIndexes));
    }

private:
    // The tree view.
    QTreeView *treeView;

    // The label that will display the selected items.
    QLabel *label;
};

class TreeSelect : public QDialog
{
xDoc doc;

public:
    TreeSelect(xDoc doc);
    ~TreeSelect();

    void AddItem(std::vector<xNode> nodes, QStandardItem *rootItem, int level);
};

TreeSelect::TreeSelect(xDoc d) {
    doc = d;
    QVBoxLayout *mainLayout = new QVBoxLayout();

    // Create the tree view.
    QTreeView *treeView = new QTreeView();
    QStandardItemModel *model = new QStandardItemModel();
    QStandardItem *rootItem = model->invisibleRootItem();


    XPathObj n = XPathObj(doc.ptr, (xmlChar*) "/*/*");
    if (n.err)
    {   // turn into macro
        std::cerr << "ERROR: " << n.err->msg->c_str();
        if (n.err->src) std::cerr << "SRC: " << n.err->src->c_str() << "\n";
        if (n.err->data) std::cerr << "QUERY: " << n.err->data->c_str() << "\n";
        return;
    }
    auto NodeList = n.Nodes();
    AddItem(NodeList, rootItem, 0);

    treeView->setModel(model);

    // Create the push button.
    QPushButton *pushButton = new QPushButton("OK");

    // Add the widgets to the layout.
    mainLayout->addWidget(treeView);
    mainLayout->addWidget(pushButton);

    // Set the layout for the dialog.
    setLayout(mainLayout);

    // Connect the signals and slots.
    // connect(pushButton, &QPushButton::clicked, this, &SelectionListDialog::accept);
    }
TreeSelect::~TreeSelect() {}

// function to recursively add xml node texts to tree browser
void TreeSelect::AddItem(std::vector<xNode> nodes, QStandardItem *rootItem, int level) {
    for (xNode node : nodes)  {
        XPathObj obj = XPathObj(node.ptr, (xmlChar*) "string(.)");
        if (!obj.err) {
            auto item = new QStandardItem((obj.Str()).c_str());
            rootItem->appendRow(item);
            auto branch = XPathObj(node.ptr, (xmlChar*) "./*");
            if (branch.results->type == XPATH_NODESET)
                {auto NS = branch.Nodes(); if (NS.size()) AddItem(NS, item, level + 1);}
            }
        }}

int main(int argc, char *argv[])
{
    int i=0; QApplication a(argc, argv, i);

    std::map<std::string, std::string> ArgList{};   //  cheap version of argparse
    for (size_t i = 1; i < argc; i++) {
        char *param = strtok(argv[i], "=")+2; char *value = strtok(NULL, "="); ArgList.insert({param, value});}

    QWidget *mainWindow = new QWidget();
    
    if (ArgList["type"].compare("filebrowser") == 0) {
        auto  FileName = QFileDialog::getOpenFileName(mainWindow, ArgList["title"].c_str(), ArgList["directory"].c_str(), ArgList["file_type"].c_str());
        if (! FileName.toStdString().length()) CANCELLED();
        std::cout <<  FileName.toStdString() << "\n"; return 0;}

    if (ArgList["type"].compare("multifiles") == 0) {
        QFileDialog dialog(mainWindow, ArgList["title"].c_str());
        dialog.setDirectory(ArgList["directory"].c_str());
        dialog.setFileMode(QFileDialog::ExistingFiles);
        dialog.setNameFilter(ArgList["file_type"].c_str());
        if (dialog.exec()) {
            QStringList  FileNames = dialog.selectedFiles();
            for ( const auto& i :  FileNames ) std::cout << i.toStdString() << " ";
            std::cout<< "\n"; return 0;}
        else CANCELLED();}

    if (ArgList["type"].compare("dirbrowser") == 0) {
        QFileDialog dialog(mainWindow, ArgList["title"].c_str());
        dialog.setDirectory(ArgList["directory"].c_str());
        dialog.setFileMode(QFileDialog::Directory);
        if (dialog.exec()) {
            QStringList DirNames = dialog.selectedFiles();
            for ( const auto& i :  DirNames ) std::cout << i.toStdString() << " ";
            std::cout<< "\n"; return 0;}
        else CANCELLED();}

    if (ArgList["type"].compare("selection") == 0) {        
        auto dialog = SelectionListDialog(mainWindow, (char*) ArgList["items"].c_str());
        dialog.setWindowTitle(ArgList["title"].c_str());
        if (dialog.exec()) {
            std::cout<< "\n"; return 0;}
        else CANCELLED();
        }

    if (ArgList["type"].compare("tree") == 0) {
        xDoc *doc;
        if (ArgList["xml"].length())
        {
            doc = new xDoc(ArgList["xml"].c_str(), "1.0", 0);
            if (doc->err)
            {   // turn into macro
                std::cerr << "ERROR: " << doc->err->msg->c_str();
                if (doc->err->src) std::cerr << "SRC: " << doc->err->src->c_str() << "\n";
                return 1;
            }
            
            XPathObj n = XPathObj(doc->ptr, (xmlChar*) "count(/selection/*)");
            if (!n.Bool()) {std::cerr << "No tree objects\n"; ERROR();}
            // n = XPathObj(doc->ptr, (xmlChar*) "string(/*/*[3])");
            n = XPathObj(doc->ptr, (xmlChar*) "/*/*");
            if (n.err)
            {   // turn into macro
                std::cerr << "ERROR: " << n.err->msg->c_str();
                if (n.err->src) std::cerr << "SRC: " << n.err->src->c_str() << "\n";
                if (n.err->data) std::cerr << "QUERY: " << n.err->data->c_str() << "\n";
                return 1;
            }
            xNode r = doc->RootNode();
            auto nn = xNode("<item>sub Item 1</item>");
            r.AddSibling(nn);
            auto i = n.Nodes();
            i[2].AddPrevSibling(nn);
            auto omg = i[2].XML();
            omg = r.XML();
            int aw = doc->XML(std::string("new.xml"));
        }
        
        auto dialog = TreeSelect(*doc);
        dialog.setWindowTitle(ArgList["title"].c_str());
        if (dialog.exec()) {
            std::cout<< "\n"; return 0;}
        else CANCELLED();
        }

    else {std::cerr << "No GUI type chosen\n"; ERROR();}
}
