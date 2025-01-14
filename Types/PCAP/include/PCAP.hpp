#pragma once

#include "Internal.hpp"

namespace GView::Type::PCAP
{
class PCAPFile : public TypeInterface
{
  public:
    Buffer data; // it's maximum 0xFFFF so just save it here

    Header header;
    std::vector<std::pair<PacketHeader*, uint32>> packetHeaders;

    PCAPFile();
    virtual ~PCAPFile()
    {
    }

    bool Update();

    std::string_view GetTypeName() override
    {
        return "PCAP";
    }
    void RunCommand(std::string_view) override
    {
    }
};

namespace Panels
{
    static ListViewItem AddMACElement(Reference<ListView> list, std::string_view name, const MAC& mac)
    {
        CHECK(list.IsValid(), ListViewItem{}, "");

        LocalString<64> tmp;
        return list->AddItem({ name.data(),
                               tmp.Format(
                                     "%02X:%02X:%02X:%02X:%02X:%02X (0x%X)",
                                     mac.arr[0],
                                     mac.arr[1],
                                     mac.arr[2],
                                     mac.arr[3],
                                     mac.arr[4],
                                     mac.arr[5],
                                     mac.value) });
    }

    static ListViewItem AddIPv4Element(Reference<ListView> list, std::string_view name, uint32 ip)
    {
        CHECK(list.IsValid(), ListViewItem{}, "");

        union
        {
            uint8 values[4];
            uint32 value;
        } ipv4{ .value = ip };

        LocalString<64> tmp;
        return list->AddItem(
              { name.data(),
                tmp.Format("%02u.%02u.%02u.%02u (0x%X)", ipv4.values[3], ipv4.values[2], ipv4.values[1], ipv4.values[0], ipv4.value) });
    }

    static ListViewItem AddIPv6Element(Reference<ListView> list, std::string_view name, uint16 ipv6[8])
    {
        CHECK(list.IsValid(), ListViewItem{}, "");

        LocalString<64> tmp;
        return list->AddItem(
              { name.data(),
                tmp.Format(
                      "%02x:%02x:%02x:%02x:%02x:%02x:%02x:%02x", ipv6[0], ipv6[1], ipv6[2], ipv6[3], ipv6[4], ipv6[5], ipv6[6], ipv6[7]) });
    }

    class Information : public AppCUI::Controls::TabPage
    {
        Reference<Object> object;
        Reference<GView::Type::PCAP::PCAPFile> pcap;
        Reference<AppCUI::Controls::ListView> general;
        Reference<AppCUI::Controls::ListView> issues;

        inline static const auto dec       = NumericFormat{ NumericFormatFlags::None, 10, 3, ',' };
        inline static const auto hexUint32 = NumericFormat{ NumericFormatFlags::HexPrefix, 16, 0, ' ', 4 };
        inline static const auto hexUint64 = NumericFormat{ NumericFormatFlags::HexPrefix, 16, 0, ' ', 8 };

        void UpdateGeneralInformation();
        void UpdatePcapHeader();
        void UpdateIssues();
        void RecomputePanelsPositions();

        void AddDateTime(std::string_view name, std::string_view format, uint64 value)
        {
            LocalString<1024> ls;
            NumericFormatter nf;
            AppCUI::OS::DateTime dt;
            dt.CreateFromFileTime(value);
            const auto valueHex = nf.ToString(value, hexUint64);
            general->AddItem({ name, ls.Format(format.data(), dt.GetStringRepresentation().data(), valueHex.data()) })
                  .SetType(ListViewItem::Type::Emphasized_1);
        }

        template <typename T>
        ListViewItem AddDecAndHexElement(std::string_view name, std::string_view format, T value)
        {
            LocalString<1024> ls;
            NumericFormatter nf;
            NumericFormatter nf2;

            static const auto hexBySize = NumericFormat{ NumericFormatFlags::HexPrefix, 16, 0, ' ', sizeof(T) * 2 };

            const auto v    = nf.ToString(value, dec);
            const auto vHex = nf2.ToString(value, hexBySize);
            return general->AddItem({ name, ls.Format(format.data(), v.data(), vHex.data()) });
        }

      public:
        Information(Reference<Object> _object, Reference<GView::Type::PCAP::PCAPFile> _pcap);

        void Update();
        virtual void OnAfterResize(int newWidth, int newHeight) override
        {
            RecomputePanelsPositions();
        }
        bool OnUpdateCommandBar(Application::CommandBar& commandBar) override;
        bool OnEvent(Reference<Control> ctrl, Event evnt, int controlID) override;
    };

    class Packets : public AppCUI::Controls::TabPage
    {
        Reference<PCAPFile> pcap;
        Reference<GView::View::WindowInterface> win;
        Reference<AppCUI::Controls::ListView> list;
        int32 Base;

        std::string_view GetValue(NumericFormatter& n, uint64 value);
        void GoToSelectedSection();
        void SelectCurrentSection();
        void OpenPacket();

      public:
        Packets(Reference<PCAPFile> _pcap, Reference<GView::View::WindowInterface> win);

        void Update();
        bool OnUpdateCommandBar(AppCUI::Application::CommandBar& commandBar) override;
        bool OnEvent(Reference<Control>, Event evnt, int controlID) override;

        class PacketDialog : public Window
        {
            Reference<GView::Object> object;
            Reference<ListView> list;
            int32 base;

            std::string_view GetValue(NumericFormatter& n, uint64 value);
            void Add_PacketHeader(LinkType type, const PacketHeader* packet);
            void Add_Package_EthernetHeader(const Package_EthernetHeader* peh, uint32 packetInclLen);
            void Add_IPv4Header(const IPv4Header* ipv4, uint32 packetInclLen);
            void Add_IPv6Header(const IPv6Header* ipv6, uint32 packetInclLen);
            void Add_UDPHeader(const UDPHeader* udp);
            void Add_DNSHeader(const DNSHeader* dns);
            void Add_ICMPHeader(const ICMPHeader_Base* icmpBase, uint32 icmpSize);
            void Add_DNSHeader_Question(const DNSHeader_Question& question);
            void Add_TCPHeader(const TCPHeader* tcp, uint32 packetInclLen);
            void Add_TCPHeader_Options(const TCPHeader* tcp, uint32 packetInclLen);

          public:
            PacketDialog(
                  Reference<GView::Object> _object,
                  std::string_view name,
                  std::string_view layout,
                  LinkType type,
                  const PacketHeader* packet,
                  int32 _base);
        };
    };
}; // namespace Panels
} // namespace GView::Type::PCAP
