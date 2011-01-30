<?php
// $Id$

/**
 * Access plugin that provides access control based on custom PHP code.
 *
 * @ingroup views_access_plugins
 */
class views_php_plugin_access extends views_plugin_access {

  /**
   * Implements views_plugin_access#summary_title().
   */
  function summary_title() {
    return t('PHP');
  }

  /**
   * Implements views_object#option_definition().
   */
  function option_definition() {
    $options = parent::option_definition();
    $options['php_access'] = array('default' => '');

    return $options;
  }

  /**
   * Implements views_plugin#options_form().
   */
  function options_form(&$form, &$form_state) {
    $form['php_access'] = array(
      '#type' => 'textarea',
      '#id' => drupal_html_id('edit-options-php-access'),
      '#title' => t('Access code'),
      '#default_value' => $this->options['php_access'],
      '#rows' => 5,
      '#description' => t('Code to construct the value of this field.') . ' <strong>' . t('Do not use &lt;?php ?&gt; delimiters.') . '</strong>',
    );
    views_php_check_input_access($form['php_access']);
    $form['php_access_variables'] = views_php_available_variables($form['php_access'], $this->view, $this, array(
      '$view_name' => t('The name of the view to check.'),
      '$display_id' => t('The ID of the display to check.'),
      '$account' => t('The account to check.'),
    ));
  }

  /**
   * Implements views_plugin_access#access().
   */
  function access($account) {
    if (!empty($this->options['php_access'])) {
      return views_php_check_access($this->options['php_access'], $this->view->name, $this->view->current_display, $account);
    }
    return TRUE;
  }

  /**
   * Implements views_plugin_access#get_access_callback().
   */
  function get_access_callback() {
    if (!empty($this->options['php_access'])) {
      return array('views_php_check_access', array($this->options['php_access'], $this->view->name, $this->view->current_display));
    }
    return TRUE;
  }
}
