/* ------------------------------------------------------------------------- */
/*  ������ ���� �������� ������ ��������� �������� �������,                  */
/*  �������������� �������� ���������� �������:                              */
/*  ������� �.�.                                                             */
/*  ������ ������������� ����������������. - �.: �����-�����, 2012. - 384 �. */
/*  ISBN 978-5-91359-102-9                                                   */
/*                                                                           */
/*  ��� � ���� �������, ����������� � ���� �������� ������ �������������     */
/*  ���� ��� ������������ � ���������� ���������������� ����������           */
/*  ������������ ��������, � ����� ��� ���������� ���������� �������������.  */
/*  ������������� ����� ���� � �������� ������ ��� �������� ��������         */
/*  ���������, ������ ������� ��������� � ����� �������������� ����          */
/*  �� ������ ����� � ���� ������������.                                     */
/*  �������� ������ ��������������� "��� ����", ��� ����� �� �� �� ����      */
/*  ����� ��� ������� �������� ����������� � ������������� ����������.       */
/*                                                                           */
/*  Copyright � 2008-2011 ������� �.�.                                       */
/* ------------------------------------------------------------------------- */


#include <windows.h>
#include "zpetri.hxx"
#include "zpetri-env.hxx"
#include "../../common/synprintf.hxx"

using namespace std;
using namespace z;
using namespace z::petri;

// ��������� ��������
struct queues_type
{
    // ...
};

// ���������� ��������
class jobprepare_type: public threadenv_type::longjob_abstract_type
{
private:
    queues_type &m_queues;
    void run(void)
    {
        synprintf(stdout, "prepare begin\n");
        // ... ���������� ����������
        ::Sleep(200);
        synprintf(stdout, "prepare end\n");
    }
public:
    jobprepare_type(queues_type &queues): m_queues(queues) {}
};

class jobget_type: public threadenv_type::longjob_abstract_type
{
    // ...
private:
    queues_type &m_queues;
    void run(void)
    {
        synprintf(stdout, "get begin\n");
        // ... ���������� ����������
        ::Sleep(200);
        synprintf(stdout, "get end\n");
    }
public:
    jobget_type(queues_type &queues): m_queues(queues) {}
};

class jobprocess_type: public threadenv_type::longjob_abstract_type
{
    // ...
private:
    queues_type &m_queues;
    void run(void)
    {
        synprintf(stdout, "process begin\n");
        // ... ���������� ����������
        ::Sleep(200);
        synprintf(stdout, "process end\n");
    }
public:
    jobprocess_type(queues_type &queues): m_queues(queues) {}
};

class jobpost_type: public threadenv_type::longjob_abstract_type
{
    // ...
private:
    queues_type &m_queues;
    void run(void)
    {
        synprintf(stdout, "post begin\n");
        // ... ���������� ����������
        ::Sleep(200);
        synprintf(stdout, "post end\n");
    }
public:
    jobpost_type(queues_type &queues): m_queues(queues) {}
};


int main(int argc, char *argv[])
{
    //~ srand(unsigned(time(NULL)));

    const int n = 10;

    // ������� ������
    queues_type queues;
    // ... ���������� ������� ������� ���������������

    // ����������� ���������� ������
    jobprepare_type jprepare(queues);
    jobget_type jget(queues);
    jobprocess_type jprocess(queues);
    jobpost_type jpost(queues);

    threadenv_type env;
    // �������
    place_type id, id1, id2, id3;
    place_type rules, state, control, result, channel;
    // ��������
    transition_simple_type split;
    threadenv_type::transition_long_type prepare(jprepare, env);
    threadenv_type::transition_long_type get(jget, env);
    threadenv_type::transition_long_type process(jprocess, env);
    threadenv_type::transition_long_type post(jpost, env);

    // ���������� ����
    petrinet_type::content_type content;
    // �������
    content.add_place(id);
    content.add_place(id1);
    content.add_place(id2);
    content.add_place(id3);
    content.add_place(rules);
    content.add_place(state);
    content.add_place(control);
    content.add_place(result);
    content.add_place(channel);
    // ��������
    content.add_transition(split);
    content.add_transition(prepare);
    content.add_transition(get);
    content.add_transition(process);
    content.add_transition(post);
    // ����
    content.add_arc(id, split);
    content.add_arc(split, id1);
    content.add_arc(split, id2);
    content.add_arc(split, id3);
    content.add_arc(id1, get);
    content.add_arc(get, state);
    content.add_arc(state, process);
    content.add_arc(id2, prepare);
    content.add_arc(prepare, rules);
    content.add_arc(rules, process);
    content.add_arc(process, control);
    content.add_arc(id3, post);
    content.add_arc(control, post);
    content.add_arc(post, result);
    content.add_arc(channel, get);
    content.add_arc(get, channel);
    content.add_arc(channel, post);
    content.add_arc(post, channel);
    // ��������
    content.add_token(id, n);
    content.add_token(channel);

    // �������� � ������ ����
    petrinet_type petrinet(content);
    petrinet.live(env);

    return 0;
}
