/********************************************************************
 * gtest-gnc-option.cpp -- unit tests for GncOption class.          *
 * Copyright (C) 2019 John Ralls <jralls@ceridwen.us>               *
 *                                                                  *
 * This program is free software; you can redistribute it and/or    *
 * modify it under the terms of the GNU General Public License as   *
 * published by the Free Software Foundation; either version 2 of   *
 * the License, or (at your option) any later version.              *
 *                                                                  *
 * This program is distributed in the hope that it will be useful,  *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of   *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the    *
 * GNU General Public License for more details.                     *
 *                                                                  *
 * You should have received a copy of the GNU General Public License*
 * along with this program; if not, contact:                        *
 *                                                                  *
 * Free Software Foundation           Voice:  +1-617-542-5942       *
 * 51 Franklin Street, Fifth Floor    Fax:    +1-617-542-2652       *
 * Boston, MA  02110-1301,  USA       gnu@gnu.org                   *
 *                                                                  *
 *******************************************************************/

#include <gtest/gtest.h>
#include <gnc-option.hpp>

TEST(GncOption, test_string_ctor)
{
    EXPECT_NO_THROW({
            GncOption option("foo", "bar", "baz", "Phony Option",
                             std::string{"waldo"});
        });
}

TEST(GncOption, test_string_classifier_getters)
{
    GncOption option("foo", "bar", "baz", "Phony Option", std::string{"waldo"});
    EXPECT_STREQ("foo", option.get_section().c_str());
    EXPECT_STREQ("bar", option.get_name().c_str());
    EXPECT_STREQ("baz", option.get_key().c_str());
    EXPECT_STREQ("Phony Option", option.get_docstring().c_str());
}

TEST(GncOption, test_string_default_value)
{
    GncOption option("foo", "bar", "baz", "Phony Option", std::string{"waldo"});
    EXPECT_STREQ("waldo", option.get_default_value<std::string>().c_str());
    EXPECT_STREQ("waldo", option.get_value<std::string>().c_str());
    EXPECT_EQ(0, option.get_value<int64_t>());
}

TEST(GncOption, test_string_value)
{
    GncOption option("foo", "bar", "baz", "Phony Option", std::string{"waldo"});
    option.set_value(std::string{"pepper"});
    EXPECT_STREQ("waldo", option.get_default_value<std::string>().c_str());
    EXPECT_NO_THROW({
            EXPECT_STREQ("pepper", option.get_value<std::string>().c_str());
        });
}

TEST(GncOption, test_int64_t_value)
{
    GncOption option("foo", "bar", "baz", "Phony Option", INT64_C(123456789));
    option.set_value(INT64_C(987654321));
    EXPECT_TRUE(option.get_default_value<std::string>().empty());
    EXPECT_EQ(INT64_C(987654321), option.get_value<int64_t>());
}

TEST(GncOption, test_string_scm_functions)
{
    GncOption option("foo", "bar", "baz", "Phony Option", std::string{"waldo"});
    auto scm_value = option.get_scm_value();
    auto str_value = scm_to_utf8_string(scm_value);
    EXPECT_STREQ("waldo", str_value);
    g_free(str_value);
    scm_value = option.get_scm_default_value();
    str_value = scm_to_utf8_string(scm_value);
    EXPECT_STREQ("waldo", str_value);
    g_free(str_value);
}

TEST(GNCOption, test_budget_ctor)
{
    auto book = qof_book_new();
    auto budget = gnc_budget_new(book);
    EXPECT_NO_THROW({
            GncOption option("foo", "bar", "baz", "Phony Option",
                             QOF_INSTANCE(budget));
        });
    gnc_budget_destroy(budget);
    qof_book_destroy(book);
}

TEST(GNCOption, test_budget_scm_functions)
{
    auto book = qof_book_new();
    auto budget = gnc_budget_new(book);
    GncOption option("foo", "bar", "baz", "Phony Option",
                     QOF_INSTANCE(budget));
    auto scm_budget = option.get_scm_value();
    auto str_value = scm_to_utf8_string(scm_budget);
    auto guid = guid_to_string(qof_instance_get_guid(budget));
    EXPECT_STREQ(guid, str_value);
    g_free(guid);
    gnc_budget_destroy(budget);
    qof_book_destroy(book);
}

TEST(GNCOption, test_commodity_ctor)
{
    auto book = qof_book_new();
    auto hpe = gnc_commodity_new(book, "Hewlett Packard Enterprise, Inc.",
                                    "NYSE", "HPE", NULL, 1);
    EXPECT_NO_THROW({
            GncOption option("foo", "bar", "baz", "Phony Option",
                             QOF_INSTANCE(hpe));
        });
    gnc_commodity_destroy(hpe);
    qof_book_destroy(book);
}
static GncOption
make_currency_option (const char* section, const char* name,
                      const char* key, const char* doc_string,
                      gnc_commodity *value)
{
    GncOption option{GncOptionValidatedValue<QofInstance*>{
        section, name, key, doc_string, QOF_INSTANCE(value),
        [](QofInstance* new_value) -> bool
            {
                return GNC_IS_COMMODITY (new_value) &&
                    gnc_commodity_is_currency(GNC_COMMODITY(new_value));
            }
        }};
    return option;
}

TEST(GNCOption, test_currency_ctor)
{
    auto book = qof_book_new();
    auto table = gnc_commodity_table_new();
    qof_book_set_data(book, GNC_COMMODITY_TABLE, table);
    auto hpe = gnc_commodity_new(book, "Hewlett Packard Enterprise, Inc.",
                                    "NYSE", "HPE", NULL, 1);
    EXPECT_THROW({
            auto option = make_currency_option("foo", "bar", "baz",
                                               "Phony Option", hpe);
        }, std::invalid_argument);
    gnc_commodity_destroy(hpe);
    auto eur = gnc_commodity_new(book, "Euro", "ISO4217", "EUR", NULL, 100);
    EXPECT_NO_THROW({
            auto option = make_currency_option("foo", "bar", "baz",
                                               "Phony Option", eur);
        });
    gnc_commodity_destroy(eur);
    auto usd = gnc_commodity_new(book, "United States Dollar",
                                 "CURRENCY", "USD", NULL, 100);
    EXPECT_NO_THROW({
            auto option = make_currency_option("foo", "bar", "baz",
                                               "Phony Option",usd);
        });
    gnc_commodity_destroy(usd);
    qof_book_set_data(book, GNC_COMMODITY_TABLE, nullptr);
    gnc_commodity_table_destroy(table);
    qof_book_destroy(book);
}

TEST(GNCOption, test_currency_setter)
{
    auto book = qof_book_new();
    auto table = gnc_commodity_table_new();
    qof_book_set_data(book, GNC_COMMODITY_TABLE, table);
    auto hpe = gnc_commodity_new(book, "Hewlett Packard Enterprise, Inc.",
                                    "NYSE", "HPE", NULL, 1);
    auto eur = gnc_commodity_new(book, "Euro", "ISO4217", "EUR", NULL, 100);
            auto option = make_currency_option("foo", "bar", "baz",
                                               "Phony Option",eur);
    auto usd = gnc_commodity_new(book, "United States Dollar",
                                 "CURRENCY", "USD", NULL, 100);
    EXPECT_NO_THROW({
            option.set_value(QOF_INSTANCE(usd));
        });
    EXPECT_PRED2(gnc_commodity_equal, usd, GNC_COMMODITY(option.get_value<QofInstance*>()));
    EXPECT_THROW({
            option.set_value(QOF_INSTANCE(hpe));
        }, std::invalid_argument);
    EXPECT_PRED2(gnc_commodity_equal, usd, GNC_COMMODITY(option.get_value<QofInstance*>()));
    gnc_commodity_destroy(hpe);
    gnc_commodity_destroy(usd);
    gnc_commodity_destroy(eur);
    qof_book_set_data(book, GNC_COMMODITY_TABLE, nullptr);
    gnc_commodity_table_destroy(table);
    qof_book_destroy(book);
}

class GncUIItem
{
public:
    void set_value(const std::string& value) { m_value = value; }
    const std::string& get_value() { return m_value; }
private:
    std::string m_value;
};

class GncOptionUITest : public ::testing::Test
{
protected:
    GncOptionUITest() :
        m_option{"foo", "bar", "baz", "Phony Option", std::string{"waldo"},
            GncOptionUIType::STRING} {}

    GncOption m_option;
};

using GncOptionUI = GncOptionUITest;

TEST_F(GncOptionUI, test_option_ui_type)
{
    EXPECT_EQ(GncOptionUIType::STRING, m_option.get_ui_type());
}

TEST_F(GncOptionUI, test_set_option_ui_item)
{
    GncUIItem ui_item;
    m_option.set_ui_item(&ui_item);
    EXPECT_EQ(&ui_item, static_cast<const GncUIItem*>(m_option.get_ui_item()));
}
