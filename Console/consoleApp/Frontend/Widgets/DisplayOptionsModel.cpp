#include "DisplayOptionsModel.h"

DisplayOptionsModel* DisplayOptionsModel::m_instance{nullptr};

DisplayOptionsModel *DisplayOptionsModel::instance()
{
    if(!m_instance){
        m_instance = new DisplayOptionsModel();
    }
    return m_instance;
}

DisplayOptionsModel::DisplayOptionsModel()
{

}
